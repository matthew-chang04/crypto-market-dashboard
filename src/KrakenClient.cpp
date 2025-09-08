#include "KrakenClient.hpp"
#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;


const std::string KrakenClient::HOST = "wss://ws.kraken.com/v2";
const std::string KrakenClient::PORT = ""; // No port

std::string KrakenClient::normalize_symbol(const std::string& symbol) {
    std::string normalized = symbol;
    for (char& ch : normalized) {
        ch = std::toupper(ch);
    }
    return normalized + "/USD";
}

void KrakenClient::subscribe_orderbook() {
    if (!beast::get_lowest_layer(*ws_).is_open()) {
        std::cerr << "Cannot subscribe with closed WebSocket";
        return;
    }

    std::string subReq = fmt::format(R"({
        "method": "subscribe",
        "params": {
            "channel": "book",
            "symbol": ["{}"]
            }
        }))", normalize_symbol(symbol_));
    
    std::shared_ptr<WebSocketClient> self = shared_from_this();
    ws_->async_write(net::buffer(subReq), 
        [self](beast::error_code& ec) {
            if (ec) {
                std::cerr << "Error sending subscription request: " << ec.message() <<std::endl;
            }

            self->ws_->async_read(self->readDump_, 
                [self](beast::error_code& ec) {
                    if (ec) {
                        std::cerr << "Error reading subscription response: " << ec.message() << std::endl;
                    } else {
                        
                    }
                }
        });
}

void KrakenClient::subscribe_ticker() {
    if 
}

std::string KrakenClient::orderbookSetup() {
    beast::basic_flat_buffer payload = readDump_.data();
    readDump_.consume(readDump_.size());
    json data = json::parse(payload);
}