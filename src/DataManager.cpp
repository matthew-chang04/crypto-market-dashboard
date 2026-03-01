#include "DataManager.hpp"
#include <chrono>
#include <string>
#include <sstream>
#include <iostream>

MarketDataManager::MarketDataManager() : latestSpotTick_{0, 0, std::chrono::system_clock::now()} {}

void MarketDataManager::addSpotTick(SpotTick tick) {
    spotTicks_.push(tick);
} {

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

void MarketDataManager::processNewTicker(const json& payload, std::chrono::system_clock::time_point timestamp) {
    std::string price_str = payload["price"];
    std::string quantity_str = payload["quantity"];

    double price = std::strtod(price_str.c_str(), nullptr);
    double quantity = std::strtod(quantity_str.c_str(), nullptr);

    SpotTick newTick{price, quantity, timestamp};
    addSpotTick(newTick);
}

void MarketDataManager::processNewOptionTick(const json& payload, std::chrono::system_clock::time_point timestamp) {
    std::string id = payload['id'].get<std::string>();
    double price = payload['price'].get<double>();
    double quantity = payload['quantity'].get<double>();
    double iv = payload['IV'].get<double>();

    OptionTick newTick{price, quantity, iv, timestamp};
    addOptionTick(newTick, id);
}

void MarketDataManager::processMessage(json payload) {

        std::string string_timestamp = payload["timestamp"];
        std::istringstream(string_timestamp);
        std::tm tm = {};

        auto stream = std::istringstream(string_timestamp);
        stream >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");

        std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::from_time_t(std::mktime(&tm));
        if (timestamp <= latestSpotTick_.timestamp) {
            return;
        }
    
    try {
        std::string type = payload["type"].get<std::string>();
        if (strcmp(type.c_str(), "ticker") == 0) {
            processNewTicker(payload, timestamp);
        } else if (strcmp(type.c_str(), "option") == 0) {
            processNewOptionTick(payload, timestamp);
        }
    } catch (const std::exception& e) {
        std::cout << "Error parsing payload: " << e.what() << std::endl;
        std::cout << "Skipping..." << std::endl;   
    }    
}


void DataManager::tick() {
	
	processPayloads();


}
