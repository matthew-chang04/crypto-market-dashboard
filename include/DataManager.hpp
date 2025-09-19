#include <string>
#include "OrderBook.hpp"
#include <unordered_map>
#include <queue>
#include <mutex>


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
    std::queue<SpotTick> spotTicks_;
    int maxSpotTicks_ = 1000; // Maximum number of spot ticks to store
    SpotTick latestSpotTick_;
    std::mutex spotMutex_;

    std::unordered_map<std::string, OptionTick> optionTicks_;
    std::mutex optionMutex_;

    OrderBook ob_;
    std::mutex obMutex_;
public:

    MarketDataManager();
    void addSpotTick(const SpotTick& tick);
    SpotTick getLatestSpotTick();

    void addOptionTick(const std::string& key, const OptionTick& tick);
    OptionTick getOptionTick(const std::string& key);

    void startOrderBook();
};