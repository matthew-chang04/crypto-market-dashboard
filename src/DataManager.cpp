#include "DataManager.hpp"
#include <chrono>
#include <string>
#include <sstream>
#include <iostream>

MarketDataManager::MarketDataManager() {}

void MarketDataManager::addSpotTick(const std::string& product, SpotTick tick) {

    if (analytics_ .contains(product)) {
        analytics_[product].update(tick);
    } else {
        analytics_[product] = AnalyticsEngine(tick);
    }
}

SpotTick MarketDataManager::getLatestSpotTick(const std::string& product) {
    std::lock_guard<std::mutex> lock(spotMutex_);

    const std::stack<SpotTick>& ticks = spotTicks_[product];

    if (ticks.size() == 0) {
        return SpotTick { 0, 0, "None", std::chrono::system_clock::now()};
    }

    return spotTicks_[product].top();
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
        tick.buyAmt = tick_event.buyAmt;
        // TODO : Add extras in, maybe a better way to scale this
        addSpotTick(tick_event.instrument, tick);
    }
}

void MarketDataManager::processMessage(MarketEvent payload) {
    processMarketEvent(std::move(payload));
}

void MarketDataManager::tick() {
}
