#include "DataManager.hpp"
#include <chrono>

bool OptionKey::operator==(const OptionKey& other) const {
    return expiry == other.expiry &&
           strike == other.strike &&
           isCall == other.isCall;
}

MarketDataManager::MarketDataManager() : latestSpotTick_{0, 0, std::chrono::system_clock::now()} {}

void MarketDataManager::addSpotTick(const SpotTick& tick) {
    std::lock_guard<std::mutex> lock(spotMutex_);
    spotTicks_.push(tick);
    if (spotTicks_.size() > maxSpotTicks_) {
        spotTicks_.pop();
    }
    latestSpotTick_ = tick;
}

SpotTick MarketDataManager::getLatestSpotTick() {
    return latestSpotTick_;
}

void MarketDataManager::addOptionTick(const OptionKey& key, const OptionTick& tick) {
    std::lock_guard<std::mutex> lock(optionMutex_);
    optionTicks_[key] = tick;
}

OptionTick MarketDataManager::getOptionTick(const OptionKey& key) {
    std::lock_guard<std::mutex> lock(optionMutex_);
    auto it = optionTicks_.find(key);
    if (it != optionTicks_.end()) {
        return it->second;
    }
    throw std::runtime_error("Option tick not found");
}