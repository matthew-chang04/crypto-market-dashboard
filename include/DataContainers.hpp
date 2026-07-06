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
};

using MarketEvent = std::variant<TickEvent>;

struct SpotTick {
    double price;
    double size;
    std::string side;
    std::chrono::system_clock::time_point timestamp;
};

struct OptionTick {
    double price;
    double quantity;
    double IV;
    std::chrono::system_clock::time_point timestamp;

	std::string expiry;
	std::string strike;
};

