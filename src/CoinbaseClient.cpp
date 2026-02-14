#include "CoinbaseClient.hpp"
#include <string>
#include <cctype>

const std::string CoinbaseClient::HOST = "wss://ws-feed.exchange.coinbase.com";
const std::string CoinbaseClient::PORT = ""; // No port

std::string CoinbaseClient::normalize_symbol(const std::string& symbol) {
    std::string normalized;
    for (char ch : symbol) {
        normalized += std::toupper(ch);
    } 
    normalized += "-USD";
    return normalized;
}

void CoinbaseClient::subscribe_orderbook() {
    if (!beast::get_lowest_layer(*ws_).is_open()) {
        std::cerr << "Cannot Connect to Closed WebSocket";
        return;
    }

    std::string subReq = fmt::format(R"({
        "type": "subscribe", 
        "channels":["level2"], 
        "product_ids": ["{}"], 
        }))", symbol_);

    ws_->async_write(net::buffer(subReq));

}

void CoinbaseClient::subscribe_ticker() {
    if (!beast::get_lowest_layer(*ws_).is_open()) {
        std::cerr << "Cannot Connect to Closed WebSocket";
        return;
    }

    std::string subReq = fmt::format(R"({
        "type": "subscribe", 
        "product_ids": ["{}"],
        "channels": ["ticker"] 
        }))", symbol_);

    ws_->async_write(net::buffer(subReq));
}

void CoinbaseClient::onMessage(const std::string& msg) {
    json payload = json::parse(msg);
    std::string type = payload["type"];
    dataManager_.addPayload(type, payload);
}