#include <map>
#include <string>
#include <thread>
#include <mutex>
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
	void run();
	void requestRestart();

	
private:
	void populateSnapshot(const json& data);
	void update();
	void prettyPrint();
 
	Exchange ex_;	
	std::unique_ptr<WebSocketClient> webSocket_;
	uint64_t lastUpdateID_;
	std::map<double, double> bids_;
	std::map<double, double> asks_;
	bool stopped_{false};
	bool restartRequested_{false};
	bool snapshotLoaded_{false};

	std::mutex obMutex_;
	std::thread updateThread_;
};
