#pragma once

#include <chrono>
#include <string>
#include <variant>

struct TickEvent {
    std::chrono::system_clock::time_point timestamp;
    std::string instrument;
    double price;
    double size;
    std::string side;

    double buyAmt;
    double sellAmt;
    double tradedAmt;
    double bestAsk;
    double bestBid;
};

struct OrderBookEvent {
    std::chrono::system_clock::time_point timestamp;
    std::string instrument;
    double bestBid;
    double bestAsk;
    
};

using MarketEvent = std::variant<TickEvent, OrderBookEvent>;

struct SpotTick {
    double price;
    double size;
    std::string side;
    std::chrono::system_clock::time_point timestamp;
 
    double buyAmt;
    double sellAmt;
    double tradedAmt;
    double bestAsk;
    double bestBid;
};

struct OptionTick {
    double price;
    double quantity;
    double IV;
    std::chrono::system_clock::time_point timestamp;

	std::string expiry;
	std::string strike;
};

