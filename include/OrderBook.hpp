#include <map>
#include <string>
#include <thread>
#include <mutex>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class OrderBook
{
public:
	OrderBook();
	void initOrderBook();
	void stop();
	void run();
	void requestRestart();

	void writeUpdate(const std::string& update);

	
private:
	void populateSnapshot(const json& data);
	void update();
	void prettyPrint();

	MarketDataManager* 
	uint64_t lastUpdateID_;
	std::map<double, double> bids_;
	std::map<double, double> asks_;
	
	bool stopped_{false};
	bool restartRequested_{false};
	bool snapshotLoaded_{false};

	std::mutex obMutex_;
	std::thread updateThread_;
};
