#include "DataManager.hpp"
#include <chrono>
#include <string>
#include <sstream>
#include <iostream>

MarketDataManager::MarketDataManager() : latestSpotTick_{0.0, 0.0, "", std::chrono::system_clock::now()} {}

void MarketDataManager::addSpotTick(SpotTick tick) {
    spotTicks_.push(tick);
}

SpotTick MarketDataManager::getLatestSpotTick() {
    return latestSpotTick_;
}

void MarketDataManager::addOptionTick(OptionTick tick, const std::string& key) {
    optionTicks_[key] = tick;
}
 
OptionTick MarketDataManager::getOptionTick(const std::string& key) {
    std::lock_guard<std::mutex> lock(optionMutex_);
    auto it = optionTicks_.find(key);
    if (it != optionTicks_.end()) {
        return it->second;
    }
    return OptionTick{0, 0, 0, std::chrono::system_clock::now()};
}

void MarketDataManager::processMarketEvent(MarketEvent payload) {
    if (std::holds_alternative<TickEvent>(payload)) {
        TickEvent tick_event = std::get<TickEvent>(payload);
        SpotTick tick;
        tick.timestamp = tick_event.timestamp;
        tick.price = tick_event.price;
        tick.size = tick_event.size;
        tick.side = tick_event.side;
        addSpotTick(tick);
    }
}

void MarketDataManager::processMessage(MarketEvent payload) {
    processMarketEvent(std::move(payload));
}

void MarketDataManager::tick() {
}
