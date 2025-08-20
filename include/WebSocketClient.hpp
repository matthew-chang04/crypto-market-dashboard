#pragma once
#include <nlohmann/json.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/ip/tcp.hpp>
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

namespace beast = boost::beast;   
namespace http = beast::http;          
namespace websocket = beast::websocket;
namespace net = boost::asio;           
using tcp = boost::asio::ip::tcp;     
using MessageHandler = std::function<void(std::string_view)>;

class WebSocketClient: public std::enable_shared_from_this<WebSocketClient>
{
public:
	WebSocketClient(net::io_context& ioc, net::ssl::context& sslCtx, tcp::resolver& resolver,  std::string target) : ioc_{ioc}, sslCtx_{sslCtx}, resolver_{resolver}, ws_{std::make_unique<websocket::stream<net::ssl::stream<tcp::socket>>>(ioc, sslCtx)}, target_{target}, interrupted_{true} {}


	virtual ~WebSocketClient() = default;

	void stop();
	void retryStart(beast::error_code ec);

	void start();	
	void do_resolve();
	void do_connect(tcp::resolver::results_type results);
	void do_ssl_handshake();
	void do_ws_handshake();
	void virtual subscribe(const std::string& target);
	void do_read();
	void reset();

	bool isInterrupted() const { return interrupted_; }
	void setInterrupted(bool value) { interrupted_ = value; }
	void setHost(std::string host) { host_ = host; }
	void setPort(std::string port) { port_ = port; }
	void setTarget(std::string target) { target_ = target; }
	void setHandler(MessageHandler handler) { on_message_ = std::move(handler); }

	std::string getTarget() const { return target_; }

protected:
	MessageHandler on_message_;
	net::io_context& ioc_;
	net::ssl::context& sslCtx_;
	tcp::resolver& resolver_;
	std::unique_ptr<websocket::stream<net::ssl::stream<tcp::socket>>> ws_;
	net::strand<net::io_context::executor_type> strand_;

	// TODO: Implement preset host, port and target values for Binance, Kraken, Coinbase
	std::string host_;
	std::string port_;
	std::string target_;

	std::mutex mutex_;
	beast::flat_buffer readDump_;
	
	bool interrupted_;
};

