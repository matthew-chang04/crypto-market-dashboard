#pragma once
#include <string>
#include <optional>
#include <condition_variable>
#include <queue>
#include <iostream>
#include <mutex>
#include <boost/beast/websocket.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>

namespace beast = boost::beast;   
namespace http = beast::http;          
namespace websocket = beast::websocket;
namespace net = boost::asio;           
using tcp = boost::asio::ip::tcp;     

class WebSocketClient
{
public:
	WebSocketClient(net::io_context& ioc, net::ssl::context sslCtx) : ws_{std::make_unique<websocket::stream<net::ssl::stream<tcp::socket>>>(ioc, sslCtx)}, interrupted_{true} {}


	virtual ~WebSocketClient() = default;

	virtual void start(std::string host, std::string port);
	virtual void subscribe(const std::string& target);
	virtual void read();
	virtual void run();
	virtual std::optional<std::string> readFromBuffer();
	virtual void stop();

	bool isInterrupted() const { return interrupted_; }
	void setInterrupted(bool value) { interrupted_ = value; }
	
	
protected:


	std::unique_ptr<websocket::stream<net::ssl::stream<tcp::socket>>> ws_;
	net::strand<net::io_context::executor_type> strand_;

	beast::flat_buffer readDump_;
	bool interrupted_;
};

