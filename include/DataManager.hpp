#include "DataContainers.hpp"
#include "AnalyticsEngine.hpp"
#include <string>
#include <unordered_map>
#include <queue>
#include <mutex>
#include <vector>
#include <nlohmann/json.hpp>

class MarketDataManager {
    bool liveBroadcastEnabled_ = true;
    std::string liveBroadcastHost_ = "127.0.0.1";
    int liveBroadcastPort_ = 8765;
    
    std::unordered_map<std::string, AnalyticsEngine> analytics_;

    std::unordered_map<std::string, MarketEvent> newPayloads_;
    std::mutex payloadMutex_;

    std::unordered_map<std::string, std::stack<SpotTick>> spotTicks_;
    std::mutex spotMutex_;
    int maxSpotTicks_ = 1000; // Maximum number of spot ticks to store

    std::unordered_map<std::string, OptionTick> optionTicks_;
    std::mutex optionMutex_;
    
    
    OrderBook ob_;
    std::mutex obMutex_;

    std::unordered_map<std::string, std::vector<InstrumentSnapshot>> analyticsHistory_;
    std::mutex analyticsHistoryMutex_;
    std::string analyticsExportPath_ = "dashboard/analytics.json";

	void processNewTicker(MarketEvent payload);
	void processNewOptionTick(MarketEvent payload);


public:

    MarketDataManager();
    void processMarketEvent(MarketEvent payload);
    void processMessage(MarketEvent payload);

    void addSpotTick(const std::string& product, SpotTick tick);
    SpotTick getLatestSpotTick(const std::string& key);

    void addOptionTick(const OptionTick& tick, const std::string& key);
    OptionTick getOptionTick(const std::string& key);

    void updateOrderBook(const OrderBookEvent& delta, const std::string& key);
    const OrderBook& getOrderBook(const std::string &key);

    void setAnalyticsExportPath(const std::string& path);
    void exportAnalyticsSnapshot(const std::string& product);
    void enableLiveBroadcast(const std::string& host, int port);

	void tick();

};
