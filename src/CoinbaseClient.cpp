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

void CoinbaseClient::subscribe_orderbook(const std::string& symbol) {
    if (!beast::get_lowest_layer(*ws_).is_open()) {
        std::cerr << "Cannot Connect to Closed WebSocket";
        return;
    }

    std::string subReq = fmt::format(R"({
        "type": "subscribe", 
        "channels":["level2"], 
        "product_ids": ["{}"], 
        }))", symbol);

    ws_->async_write(net::buffer(subReq));

}

void CoinbaseClient::subscribe_ticker(const std::string& symbol, const std::string& interval) {
    if (!beast::get_lowest_layer(*ws_).is_open()) {
        std::cerr << "Cannot Connect to Closed WebSocket";
        return;
    }

    std::string subReq = fmt::format(R"({
        "type": "subscribe", 
        "product_ids": ["{}"],
        "channels": ["ticker"] 
        }))", symbol);

    ws_->async_write(net::buffer(subReq));
}