#include "CoinbaseClient.hpp"
#include <string>
#include <cctype>
#include <sstream>
#include <iomanip>
#include <ctime>

const std::string CoinbaseClient::HOST = "ws-feed-public.sandbox.exchange.coinbase.com";
const std::string CoinbaseClient::PORT = "443";

constexpr const char* subTemplate = R"({{
        "type": "subscribe", 
        "channels":[
            {{            
                "name" : "level2",
                "product_ids": ["{0}"]
            }}
        ]
    }})";

constexpr const char* unsubTemplate = R"({{
        "type": "unsubscribe",
        "channels":[
            {{            
                "name" : "level2",
                "product_ids": ["{0}"]
            }}
        ]   
    }})";
    
const std::string& CoinbaseClient::normalizeSymbol(const std::string& symbol) {
    static std::string normalized;
    normalized = symbol;
    std::transform(normalized.begin(), normalized.end(), normalized.begin(), [](unsigned char c) {
        return static_cast<char>(std::toupper(c));
    });
    normalized += "-USD";
    return normalized;
}

std::optional<MarketEvent> CoinbaseClient::parsePayload(const std::string& msg) {
    try {
        nlohmann::json payload = nlohmann::json::parse(msg);

        std::cout << payload << std::endl;
        std::string type = payload["type"].get<std::string>();

        if (type == "ticker") {
            TickEvent event{};

            std::string string_timestamp = payload["time"].get<std::string>();

            if (!string_timestamp.empty() && string_timestamp.back() == 'Z') {
                string_timestamp.pop_back();
            }
            auto fractional_sec = string_timestamp.find('.');
            if (fractional_sec != std::string::npos) {
                string_timestamp = string_timestamp.substr(0, fractional_sec);
            }

            std::cout << "Timestamp string: " << string_timestamp << std::endl;
            
            std::tm tm = {};
            std::istringstream stream(string_timestamp);

            std::cout << "did get time run?" << std::endl;
            stream >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
            std::cout << "get time ran successfully" << std::endl;
            if (!stream || stream.fail()) {
                std::cerr << "Failed to parse timestamp: " << string_timestamp << std::endl;
                return std::nullopt;
            }

            #ifdef _WIN32
            time_t seconds = _mkgmtime(&tm);
            #else
            time_t seconds = timegm(&tm);
            #endif
            if (seconds == -1) {
                std::cerr << "Failed to convert UTC time: " << string_timestamp << std::endl;
                return std::nullopt;
            }

            std::cout << "UTC seconds: " << seconds << std::endl;
            event.timestamp = std::chrono::system_clock::from_time_t(seconds);

            std::cout << "timestamp created" << std::endl;
            event.instrument = payload["product_id"].get<std::string>();

            std::cout << "produdct _id " << event.instrument << std::endl;
            std::string price_str = payload["price"].get<std::string>();
            std::string size_str = payload["last_size"].get<std::string>();

            std::cout << "price and size" << price_str << " " << size_str << std::endl;
            event.price = std::stod(price_str);
            event.size = std::stod(size_str);
            event.side = payload["side"].get<std::string>();

            return std::make_optional(MarketEvent{event});
        } else {
            return std::nullopt;
        }
    } catch (const std::exception& e) {
        std::cerr << "Coinbase parse error: " << e.what() << std::endl;
        return std::nullopt;
    }
}

std::string CoinbaseClient::buildRequestMsg(const std::string& action, const std::string& product) {
    
    nlohmann::json j;
    j["type"] = "subscribe";
    j["channels"] = nlohmann::json::array();
    j["channels"].push_back({
        {"name", "heartbeat"},
        {"product_ids", nlohmann::json::array({ product })}
    });

    return j.dump();
}