#include "DataContainers.hpp"
#include <string>
#include <unordered_map>
#include <queue>
#include <mutex>
#include <nlohmann/json.hpp>

class MarketDataManager {
	
	std::string ticker;
    std::unordered_map<std::string, MarketEvent> newPayloads_;
    std::mutex payloadMutex_;

    std::queue<SpotTick> spotTicks_;
    std::mutex spotMutex_;
    int maxSpotTicks_ = 1000; // Maximum number of spot ticks to store
    SpotTick latestSpotTick_;

    std::unordered_map<std::string, OptionTick> optionTicks_;
    std::mutex optionMutex_;

	void processNewTicker(MarketEvent payload);
	void processNewOptionTick(MarketEvent payload);
public:

    MarketDataManager();
    void processMarketEvent(MarketEvent payload);

    void addSpotTick(SpotTick tick);
    SpotTick getLatestSpotTick();

    void addOptionTick(OptionTick tick, const std::string& key);
    OptionTick getOptionTick(const std::string& key);

	void tick();

};
