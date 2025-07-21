
#include "BinanceClient.hpp"
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <iostream>
#include <format>

#define HOST "stream.binance.com"
#define PORT "9443"

namespace json = nlohmann::json;
namespace beast = boost::beast;        
namespace http = beast::http;           
namespace websocket = beast::websocket; 
namespace net = boost::asio;           
using tcp = boost::asio::ip::tcp;       

//TODO: SEE about adding enums to standardize names for coins (ie BTC, ETH, SOL)
// THIS WILL allow us to send in specific symbols. probably worth implementing some enum in the WebSocketClient Class, and passing symbol from there...

BinanceClient::BinanceClient(std::string symbol) : WebSocketClient(HOST, PORT) {}

void connect()
{
	auto const results = resolver_.resolve(host_, port_);
	auto ep = net::connect(ws_.next_layer(), results);

	host_ += ":" + std::to_string(ep.port());
	ws_.set_option(websocket::stream::base::decorator(
		[](websocket::request_type& req)
		{
			req.set(http::field::user_agent, std::string(BOOST_BEAST_VERSION_STRING) + " websocket-client-coro");
		}));

	ws_.handshake(host_, "/");
}

void subscribe(std::string target)
{
	std::string subReq = std::format("{ \"method\": \"SUBSCRIBE\", \"params\": [ {} ], \"id\" : 1", target);

	ws_.write(net::buffer(subReq);
}

void run()
{
	beast::flat_buffer buffer;
	while (true) {
		ws_.read(buffer);
		std::string payload = beast::buffer_to_string(buffer.data());
		buffer.consume(buffer.size());
		// IMPLEMENT A FUNCTION THAT TAKES THE PAYLOAD AND DEALS WITH Ig
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
		
		beast::tcp_stream stream(ioc_, sslCtx_);
		auto const results = resolver_.resolve(API_HOST, API_PORT);
		beast::get_lowest_layer(stream).connect(results);
		stream.handshake(ssl::stream_base::client);

		http::request<http::string_body> req{http::verb::get, API_END + target, version};
		req.set(http::field::host, API_HOST);
		req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

		http::write(stream, req);

		beast::flat_buffer buffer;
		http::response<http::dynamic_body> result;
		http::read(stream, buffer, result);

		return beast::result_buffers_to_string(result.body().data());
	} catch(std::exception const& e) {
		std::cerr << "ERROR: " << e.what() << std::endl;
		return "";
	}	
}

void initOrderbook()
{
	// LOGIC: must implement a message handle function that stores stream info in a buffer that is a member std::vector, this will hold the updates while we wait for the right snapshot

}
