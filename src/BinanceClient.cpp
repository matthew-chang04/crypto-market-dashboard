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


void BinanceClient::subscribe(const std::string& target) {
	if (!beast::get_lowest_layer(*ws_).is_open()) {
		std::cerr << "Cannot Connect to Closed WebSocket";
		return;
	}

	std::string subReq = fmt::format(R"({{ "method": "SUBSCRIBE", "params": [ "{}" ], "id" : 1 }})", target);
	ws_->async_write(net::buffer(subReq));
	std::shared_ptr<WebSocketClient> self = shared_from_this();
	boost::asio::bind_executor(strand_, 
		[self, target](beast::error_code ec, std::size_t bytes_written) {
			if (ec) {
				std::cerr << "Subscription to target: " << target << "failed." << std::endl;
				return;
			}
			std::cout << "Subscription to " << target << "sucessful." << std::endl;
			self->setInterrupted(false);
			self->do_read();
		});
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
