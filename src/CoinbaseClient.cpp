#include "CoinbaseClient.hpp"
#include <string>
#include <cctype>

const std::string CoinbaseClient::HOST = "ws-feed-public.sandbox.exchange.coinbase.com";
const std::string CoinbaseClient::PORT = "443";

constexpr const char* subTemplate = R"({{
        "type": "subscribe", 
        "channels":["level2"], 
        "product_ids": ["{0}"], 
    }})";

constexpr const char* unsubTemplate = R"({{
        "type": "unsubscribe", 
        "channels":["level2"], 
        "product_ids": ["{0}"], 
    }})";
    
std::string CoinbaseClient::normalize_symbol(const std::string& symbol) {
    std::string normalized;
    for (char ch : symbol) {
        normalized += std::toupper(ch);
    } 
    normalized += "-USD";
    return normalized;
}

void CoinbaseClient::subscribe_orderbook(const std::string& symbol) {
    if (!beast::get_lowest_layer(*ws_).is_open()) {
        std::cerr << "Cannot Connect to Closed WebSocket";
        return;
    }

    std::string subReq = fmt::format(subTemplate, symbol);

    ws_->async_write(net::buffer(subReq));

}

void CoinbaseClient::subscribe_ticker(const std::string& symbol) {
    if (!beast::get_lowest_layer(*ws_).is_open()) {
        std::cerr << "Cannot Connect to Closed WebSocket";
        return;
    }

    std::string subReq = fmt::format(subTemplate, symbol);

    ws_->async_write(net::buffer(subReq));
} 

void CoinbaseClient::unsubscribe_ticker(const std::string& symbol) {
    if (!beast::get_lowest_layer(*ws_).is_open()) {
        std::cerr << "Cannot Communicate with Closed WebSocket";
        return;
    }

    std::string unsubReq = fmt::format(unsubTemplate, symbol);

    ws_->async_write(net::buffer(unsubReq));
}

nlohmann::json CoinbaseClient::parsePayload(const std::string& msg) {
    nlohmann::json payload = nlohmann::json::parse(msg);
    std::string type = payload["type"].get<std::string>();

    nlohmann::json normalized;
    if (strcmp(type.c_str(), "ticker") == 0) {
        normalized['type'] = 'ticker';
        normalized["symbol"] = payload["product_id"];
        normalized["price"] = std::stod(payload["price"].get<std::string>());
        normalized["side"] = payload["side"];
    }

    return normalized;
}
