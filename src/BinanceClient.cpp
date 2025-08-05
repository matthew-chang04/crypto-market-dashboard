#include "BinanceClient.hpp"
#include <nlohmann/json.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/beast/ssl.hpp> 
#include <optional>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp> // verify this
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/core/buffers_to_string.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <string>
#include <optional>
#include <condition_variable>
#include <thread>
#include <iostream>
#include <mutex>
#include <queue>
#include <fmt/format.h>
#include <stdexcept>
#include <chrono>

namespace http = beast::http;           
namespace websocket = beast::websocket; 
namespace net = boost::asio;           
using tcp = boost::asio::ip::tcp;      
using json = nlohmann::json;


//TODO: SEE about adding enums to standardize names for coins (ie BTC, ETH, SOL)
// THIS WILL allow us to send in specific symbols. probably worth implementing some enum in the WebSocketClient Class, and passing symbol from there...

const std::string BinanceClient::HOST = "stream.binance.com";
const std::string BinanceClient::PORT = "9443";

void BinanceClient::connect()
{
    const int MAX_RETRIES = 10;
    int retries = 0;
    while (retries < MAX_RETRIES) {
		std::cout << "Attempting to connect to " << host_ << ":" << port_ << " (Attempt " << retries + 1 << ")" << std::endl;
        try {
            if (ws_->is_open()) {
                ws_->close(websocket::close_code::normal);
            }

			if (retries > 0) {
				ws_ = std::make_unique<websocket::stream<net::ssl::stream<tcp::socket>>>(ioc_, sslCtx_);

				auto base_delay = std::min(1000 * (1 << (retries - 1)), 30000); 
                auto jitter = std::rand() % 1000; // 
                std::this_thread::sleep_for(std::chrono::milliseconds(base_delay + jitter));
			}
            
            auto const results = resolver_.resolve(host_, port_);
            auto ep = net::connect(beast::get_lowest_layer(*ws_), results);

			if (!SSL_set_tlsext_host_name(ws_->next_layer().native_handle(), host_.c_str())) {
				beast::error_code ec(static_cast<int>(::ERR_get_error()), net::error::get_ssl_category());
				throw beast::system_error(ec);
			}

            ws_->next_layer().set_verify_callback(net::ssl::host_name_verification(host_));
            
            std::string handshakeHost = host_ + ":" + std::to_string(ep.port());

            ws_->next_layer().handshake(net::ssl::stream_base::client);

            ws_->set_option(websocket::stream_base::timeout::suggested(
                beast::role_type::client));

            ws_->set_option(websocket::stream_base::decorator(
                [](websocket::request_type& req) {
                    req.set(http::field::user_agent,
                        std::string(BOOST_BEAST_VERSION_STRING) + " websocket-client-coro");
                }));

            ws_->handshake(handshakeHost, "/stream");

			std::cout << "Connected successfully to " << handshakeHost << std::endl;
            return;

        } catch (const boost::beast::system_error& be) {
            std::cerr << "Beast error (attempt " << retries + 1 << "): " 
                     << be.code() << ": " << be.what() << std::endl;
            
			retries++;
            
            if (retries == MAX_RETRIES) {
                throw;
            }            
        } catch (const std::exception& e) {
            std::cerr << "Standard error: " << e.what() << std::endl;
            throw;
        }
    }
}

void BinanceClient::subscribe(const std::string& target)
{
	if (!beast::get_lowest_layer(*ws_).is_open()) {
		throw std::invalid_argument("Cannot Subscribe using closed websocket");
	}

	try {
		std::string subReq = fmt::format(R"({{ "method": "SUBSCRIBE", "params": [ "{}" ], "id" : 1 }})", target);
		ws_->write(net::buffer(subReq));
		
		ws_->read(readDump_);
		std::string payload = boost::beast::buffers_to_string(readDump_.data());
		readDump_.consume(readDump_.size());

		json response = json::parse(payload);
		if (response["result"].is_null()) {
			std::cout << "Subscription successful for target: " << target << std::endl;
		}

	} catch (const std::exception& ec) {
		std::cerr << "WebSocket Subscribe Error: " << ec.what() << std::endl;
		throw;
	}

}

void BinanceClient::read()
{
	ws_->async_read(readDump_, [this](beast::error_code ec, size_t bytes) {
		if (!ec) {
			std::string payload = boost::beast::buffers_to_string(readDump_.data());
			readDump_.consume(readDump_.size());
			buffer_.push(payload);
			read();
		} else {
			std::cerr << "WebSocket Read Error: " << ec.message() << std::endl;
			throw std::runtime_error("WebSocket read failed");
		}
	});
}

void BinanceClient::run()
{
	read();
	std::thread([this]() {
		ioc_.run();
	}).detach();
}

std::optional<std::string> BinanceClient::readFromBuffer()
{
	if (buffer_.empty()) {
		return std::nullopt;
	}
	std::string payload = buffer_.front();
	buffer_.pop();
	return payload;
}

void BinanceClient::stop()
{
	ws_->close(websocket::close_code::normal);
	ioc_.stop( );
	readDump_.consume(readDump_.size());
	while (!buffer_.empty()) {
		buffer_.pop();
	}
}

// TODO: Implement some way that data gets passed to here properly (proper target format @BTCUSDT/limit=100
std::string BinanceClient::getOrderBookSnapshot(const std::string& target)
{
	try {
		const std::string API_HOST = "api.binance.com";
		const std::string API_PORT = "443";
		const std::string API_END = "/api/v3/depth?symbol=";
		const int version = 11; 
		
		net::io_context ioc;
		net::ssl::context sslCtx{boost::asio::ssl::context::tlsv12_client};
		tcp::resolver resolver{ioc};

		net::ssl::stream<beast::tcp_stream> stream(ioc, sslCtx);
		auto const results = resolver.resolve(API_HOST, API_PORT);
		beast::get_lowest_layer(stream).connect(results);
		stream.handshake(net::ssl::stream_base::client);

			// TODO: MAKE THIS SCALABLE (WE ARE HARD CODING BTCUSDT)
		http::request<http::string_body> req{http::verb::get, API_END + "BTCUSDT", version};
		req.set(http::field::host, API_HOST);
		req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

		http::write(stream, req);

		beast::flat_buffer buffer;
		http::response<http::dynamic_body> result;
		http::read(stream, buffer, result);

		return boost::beast::buffers_to_string(result.body().data());
	} catch(std::exception const& e) {
		std::cerr << "ERROR: " << e.what() << std::endl;
		return "";
	}	
}

