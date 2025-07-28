#include <string>
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

	WebSocketClient(const std::string& host, const std::string& port) : host_{host}, port_{port}, ioc_{}, sslCtx_{boost::asio::ssl::context::tlsv13_client}, resolver_{ioc_}, ws_{ioc_, sslCtx_}, buffer_{}, readDump_{} {} 
	virtual ~WebSocketClient() {}

	virtual void connect() = 0;
	virtual void subscribe(const std::string& target);
	virtual void read() = 0;
	virtual void run() = 0;
	virtual std::string readFromBuffer() = 0;
	virtual void stop() = 0;

	bool bufferIsEmpty() const
	{
		return buffer_.empty();
	}
	
	// TODO: Implement the disconnect logic, make sure the web socket closes cleanly (veryify if this should be done on the websocket or derrive class level
protected:
	std::string host_;
	std::string port_;

	net::io_context ioc_;
	net::ssl::context sslCtx_;
	tcp::resolver resolver_;
	websocket::stream<net::ssl::stream<tcp::socket>> ws_;

	std::queue<std::string> buffer_;
	beast::flat_buffer readDump_;
};

