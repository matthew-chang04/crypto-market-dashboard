#include <string>
#include <optional>
#include <condition_variable>
#include <queue>
#include <iostream>
#include <boost/beast/websocket.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>

namespace beast = boost::beast;   
namespace http = beast::http;          
namespace websocket = beast::websocket;
namespace net = boost::asio;           
using tcp = boost::asio::ip::tcp;     

class WebSocketClient
{
public:

	WebSocketClient(const std::string& host, const std::string& port) : host_{host}, port_{port}, ioc_{}, sslCtx_{boost::asio::ssl::context::tlsv12_client}, resolver_{ioc_}, ws_{}, buffer_{}, readDump_{}
{
	try {
		sslCtx_.set_default_verify_paths();
		sslCtx_.set_verify_mode(net::ssl::verify_peer);
		ws_ = std::make_unique<websocket::stream<net::ssl::stream<tcp::socket>>>(ioc_, sslCtx_);
	} catch (const std::exception& e) {
		std::cerr << "Error initializing WebSocketClient: " << e.what() << std::endl;
		throw;
	}
} 

	virtual ~WebSocketClient() = default;

	virtual void connect() = 0;
	virtual void subscribe(const std::string& target) = 0;
	virtual void read() = 0;
	virtual void run() = 0;
	virtual std::optional<std::string> readFromBuffer() = 0;
	virtual void stop() = 0;

	bool bufferIsEmpty() const
	{
		return buffer_.empty();
	}
	
protected:
	std::string host_;
	std::string port_;

	net::io_context ioc_;
	net::ssl::context sslCtx_;
	tcp::resolver resolver_;
	std::unique_ptr<websocket::stream<net::ssl::stream<tcp::socket>>> ws_;

	std::queue<std::string> buffer_;
	beast::flat_buffer readDump_;
};

