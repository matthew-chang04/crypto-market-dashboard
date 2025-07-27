#include <map>
#include <string>

enum class Exchange
{
	Binance,
	Kraken,
	Coinbase
};

class OrderBook
{
public:
	OrderBook(Exchange ex, std::unique_ptr<WebSocketClient> websocket);
	void initOrderBook();
	void stop();
	void testLoop();
	
private:
	void populateSnapshot(const std::string& response);
	void update(const std::string& jsonUpdate);
	void testRun();

	Exchange ex_;	
	std::unique_ptr<WebSocketClient> webSocket_;
	uint64_t lastUpdateID_;
	std::map<double, double> bids_;
	std::vector<double> bidsUpdated_;

	std::map<double, double> asks_;
	std::vector<double> asksUpdated_;
	bool stopped_; 

	std::mutex obMutex_;
};
