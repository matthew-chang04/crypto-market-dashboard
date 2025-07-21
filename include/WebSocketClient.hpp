
#include <string>

class WebSocketClient
{
public:
	
	WebSocketClient(std::string& host, std::string& port) : host_ {host}, port_{port}, ioc_{}, sslCtx_{boost::asio::ssl::context::tlsv12_client}, resolver_{ioc_}, ws_{ioc_, sslCtx_} {} 
	virtual ~WebSocketClient() {}

	void connect() = 0;
	void subscribe(const std::string& target);
	void run() = 0;

protected:
	std::string host_;
	std::string port_;

	boost::asio::io_context ioc_;
	boost::asio::ssl::context sslCtx_;
	tcp::resolver resolver_;
	websocket::stream::<ssl::stream<tcp::socket>> ws_;
};

