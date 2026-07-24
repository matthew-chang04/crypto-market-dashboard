#pragma once

#include <chrono>
#include <string>
#include <variant>
#include <map>

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
    std::unordered_map<double, double> newBids;
    std::unordered_map<double, double> newAsks;

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

struct OrderBook {
    std::map<double, double> bids_;
    std::map<double, double> asks_;

    double spread_;
    double depth_;
    double imbalance_;
    double microprice_;
    double vwap_;
    
};
struct OptionTick {
    double price;
    double quantity;
    double IV;
    std::chrono::system_clock::time_point timestamp;

	std::string expiry;
	std::string strike;
};

