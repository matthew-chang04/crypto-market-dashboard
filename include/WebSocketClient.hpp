
#include <string>

class WebSocketClient
{
public:
	
	WebSocketClient(std::string& uri) : uri_ {uri} {}
	virtual ~WebSocketClient() {}

	void connect();
	void run();

private:
	virtual void on_message(const std::string& msg) = 0;
	virtual void subscribe() = 0;

	std::string uri_;
};

