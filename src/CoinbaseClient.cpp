#include "CoinbaseClient.hpp"
#include <string>
#include <cctype>

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
    std::string normalized;
    for (char ch : symbol) {
        normalized += std::toupper(ch);
    } 
    normalized += "-USD";
    return normalized;
}

std::optional<MarketEvent> CoinbaseClient::parsePayload(const std::string& msg) {
    try {

        nlohmann::json payload = nlohmann::json::parse(msg);

        std::cout << payload << std::endl;
        std::string type = payload["type"].get<std::string>();

        std::cout << type << std::endl;
        if (type == "ticker") {
            TickEvent event { };

            std::string string_timestamp = payload["timestamp"].get<std::string>();
            std::tm tm = {};

            auto stream = std::istringstream(string_timestamp);
            stream >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");

            std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::from_time_t(std::mktime(&tm));

            event.timestamp = timestamp;
            event.instrument = payload["product_id"].get<std::string>();
            
            std::string price_str = payload["price"].get<std::string>();
            std::string size_str = payload["last_size"].get<std::string>();

            event.price = std::stod(price_str);
            event.size = std::stod(size_str);
            event.side = payload["side"];
        } else {
            return std::nullopt;
        }
         
    } catch (const std::exception& e) {
        return std::nullopt;
    }
}

const nlohmann::json& CoinbaseClient::buildRequestMsg(const std::string& action, const std::string& product) {
    
    std::string symbol = normalizeSymbol(product);
    nlohmann::json j;
    j["type"] = "subscribe";
    j["channels"] = nlohmann::json::array();
    j["channels"].push_back({
        {"name", "ticker"},
        {"product_ids", nlohmann::json::array({ symbol })}
    });

    return j.dump();
}