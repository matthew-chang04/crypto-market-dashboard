#include "DataManager.hpp"
#include <chrono>
#include <string>
#include <sstream>
#include <iostream>

MarketDataManager::MarketDataManager() : latestSpotTick_{0, 0, std::chrono::system_clock::now()} {}

void MarketDataManager::addSpotTick(json payload) {

}

SpotTick MarketDataManager::getLatestSpotTick() {
    return latestSpotTick_;
}

void MarketDataManager::addOptionTick(json payload) {

    std::lock_guard<std::mutex> lock(optionMutex_);
    optionTicks_[key] = tick;

	// TODO: fix client manager and data manager relations, should client manager just send payloads, or parse them to be pulled by data manager?? 

}


OptionTick MarketDataManager::getOptionTick(const std::string& key) {
    std::lock_guard<std::mutex> lock(optionMutex_);
    auto it = optionTicks_.find(key);
    if (it != optionTicks_.end()) {
        return it->second;
    }
    return OptionTick{0, 0, 0, std::chrono::system_clock::now()};
}

// TODO: Accomodate multiple exchanges (1 more layer of normalization (probably on the client side))
void MarketDataManager::processMessage(json payload) {
    std::string type = payload["type"].get<std::string>();
    if (strcmp(type.c_str(), "ticker") == 0) {
        std::string string_timestamp = payload["timestamp"];
        std::istringstream(string_timestamp);
        std::tm tm = {};

        auto stream = std::istringstream(string_timestamp);
        stream >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");

        std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::from_time_t(std::mktime(&tm));
        if (timestamp <= latestSpotTick_.timestamp) {
            continue;
        }

        try {
            std::string price_str = payload["price"];
            std::string quantity_str = payload["quantity"];

            double price = std::strtod(price_str.c_str(), nullptr);
            double quantity = std::strtod(quantity_str.c_str(), nullptr);

            SpotTick newTick{price, quantity, timestamp};
            addSpotTick(newTick);

        } catch (const std::exception& e) {
            std::cout << "Error parsing payload: " << e.what() << std::endl;
            std::cout << "Skipping..." << std::endl;
        }
    }        
}


void DataManager::tick() {
	
	processPayloads();


}
