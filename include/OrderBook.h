#include <map>
#include <string>

enum class Exchange
{
	Binance,
	Kraken,
	Coinbase
}

class OrderBook
{
public:
	OrderBook(Exchange ex, std::unique_ptr<WebSocketClient> websocket) {}	
	void populateSnapshot(const std::string& response) {}
	void update(const std::string& jsonUpdate) {}
	void initOrderBook() {}
	

private:
	Exchange ex_;	
	std::unique_ptr<WebSocketClient> webSocket_;
	uint64_t lastUpdateID_;
	std::map<double, double> bids_;
	std::map<double, double> asks_;
};
