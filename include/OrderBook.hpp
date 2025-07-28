#include <map>
#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

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
	void populateSnapshot(const json& data);
	void update();
	void testRun();

	Exchange ex_;	
	std::unique_ptr<WebSocketClient> webSocket_;
	uint64_t lastUpdateID_;
	std::map<double, double> bids_;
	std::map<double, double> asks_;
	bool stopped_; 

	std::mutex obMutex_;
};
