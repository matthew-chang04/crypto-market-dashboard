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
};

class MarketDataManager {
    std::unordered_map<std::string, json> newPayloads_;
    std::mutex payloadMutex_;

    std::queue<SpotTick> spotTicks_;
    int maxSpotTicks_ = 1000; // Maximum number of spot ticks to store
    SpotTick latestSpotTick_;

    std::unordered_map<std::string, OptionTick> optionTicks_;

    OrderBook ob_;
public:

    MarketDataManager();
    void addPayload(const std::string& type, const json& payload);

    void addSpotTick(const SpotTick& tick);
    SpotTick getLatestSpotTick();

    void addOptionTick(const std::string& key, const OptionTick& tick);
    OptionTick getOptionTick(const std::string& key);

    void startOrderBook();
    std::shared_ptr<OrderBook> getOrderBook() { return std::make_shared<OrderBook>(ob_); }


};
