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

struct OptionKey {
    std::string expiry; // Format: "DDMMMYY"
    double strike;
    bool isCall;
    
    // We could just standardize the options keys to be the same as deribit, since this is our only options source
    bool operator==(const OptionKey& other) const;
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

    std::unordered_map<OptionKey, OptionTick> optionTicks_;
    std::mutex optionMutex_;

    OrderBook ob_;
    std::mutex obMutex_;
public:

    MarketDataManager();
    void addSpotTick(const SpotTick& tick);
    SpotTick getLatestSpotTick();

    void addOptionTick(const OptionKey& key, const OptionTick& tick);
    OptionTick getOptionTick(const OptionKey& key);

    void startOrderBook();
};