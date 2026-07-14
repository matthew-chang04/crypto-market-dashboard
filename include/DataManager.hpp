#include "DataContainers.hpp"
#include "AnalyticsEngine.hpp"
#include <string>
#include <unordered_map>
#include <queue>
#include <mutex>
#include <vector>
#include <nlohmann/json.hpp>

class MarketDataManager {
    
    std::unordered_map<std::string, AnalyticsEngine> analytics_;

    std::unordered_map<std::string, MarketEvent> newPayloads_;
    std::mutex payloadMutex_;

    std::unordered_map<std::string, std::stack<SpotTick>> spotTicks_;
    std::mutex spotMutex_;
    int maxSpotTicks_ = 1000; // Maximum number of spot ticks to store

    std::unordered_map<std::string, OptionTick> optionTicks_;
    std::mutex optionMutex_;

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

    void addOptionTick(OptionTick tick, const std::string& key);
    OptionTick getOptionTick(const std::string& key);

    void setAnalyticsExportPath(const std::string& path);
    void exportAnalyticsSnapshot(const std::string& product);

	void tick();

};
