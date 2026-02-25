#include <string>
#include "OrderBook.hpp"
#include <unordered_map>
#include <queue>
#include <mutex>
#include <nlohmann/json.hpp>


struct SpotTick {
    double price;
    double quantity;
    std::chrono::system_clock::time_point timestamp;
};


struct OptionTick {
    double price;
    double quantity;
    double IV;
    std::chrono::system_clock::time_point timestamp;

	std::string expiry;
	std::string strike;

};

class MarketDataManager {
	
	std::string ticker;
    std::unordered_map<std::string, json> newPayloads_;
    std::mutex payloadMutex_;

    std::queue<SpotTick> spotTicks_;
    std::mutex spotMutex_;
    int maxSpotTicks_ = 1000; // Maximum number of spot ticks to store
    SpotTick latestSpotTick_;

    std::unordered_map<std::string, OptionTick> optionTicks_;
    std::mutex optionMutex_;

    OrderBook ob_;
public:

    MarketDataManager();
    void processMessage(nlohmann::json payload);

    void addSpotTick(nlohmann::json payload);
    SpotTick getLatestSpotTick();

    void addOptionTick(nlohmann::json payload);
    OptionTick getOptionTick(const std::string& key);

    void startOrderBook();
    std::shared_ptr<OrderBook> getOrderBook() { return std::make_shared<OrderBook>(ob_); }

	void tick();

};
