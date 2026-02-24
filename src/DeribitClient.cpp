#include "WebSocketClient.hpp"
#include "DeribitClient.hpp"
#include "DataManager.hpp"
#include <nlohmann/json.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/beast/ssl.hpp> 
#include <optional>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp> // verify this
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/core/buffers_to_string.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <string>
#include <optional>
#include <condition_variable>
#include <thread>
#include <iostream>
#include <mutex>
#include <queue>
#include <fmt/format.h>
#include <stdexcept>
#include <chrono>

namespace http = beast::http;           
namespace websocket = beast::websocket; 
namespace net = boost::asio;           
using tcp = boost::asio::ip::tcp;      
using json = nlohmann::json;

const std::string DeribitClient::HOST = "wss://www.deribit.com/ws/api/v2";
const std::string DeribitClient::PORT = "";

DeribitClient::DeribitClient(net::io_context& ioc, net::ssl::context& sslCtx, tcp::resolver& resolver, std::string target, std::string symbol, MarketDataManager& dataManager)
        : WebSocketClient(ioc, sslCtx, resolver, target, symbol, dataManager) {
            setHost(HOST);
            setPort(PORT);     
            
            auto now = floor<std::chrono::days>(system_clock::now());   
            std::chrono::sys_days day = time_point_cast<std::chrono::days>(now);

        int businessDaysCount = 0;

        while (businessDaysCount < 10) {
            // Get weekday in UTC
            weekday wd{day};
            if (wd != Sunday && wd != Saturday) {
                trackedExpiries_.push_back(format_date(day));
                ++businessDaysCount;
            }
            day += days{1};
        }
 }

std::string DeribitClient::normalize_symbol(const std::string& symbol) {
    std::string normalized(symbol);
    std::transform(normalized.begin(), normalized.end(), normalized.begin(), ::toupper);
    return normalized;
}

std::string DeribitClient::format_date(std::chrono::sys_days day) {

    // Format date as DDMMMYY (e.g., 30JUN23)
     std::string date = std::format("{:%d%b%y}", day);

    for (size_t i = 2; i < 5; ++i) {
        date[i] = std::toupper(date[i]);
    }
    return date;
}

std::string DeribitClient::create_symbol(const std::string& base, const std::string& expiry, double strike) {    
    const std::string symbol = fmt::format("{}-{}-{}-C", base, expiry, static_cast<int>(strike));
    // example: BTC-30JUN23-30000-C
    return symbol;
}

void DeribitClient::subscribe_ticker(const std::string& symbol) {
    if (!beast::get_lowest_layer(*ws_).is_open()) {
        std::cerr << "Cannot subscribe with closed WebSocket";
        return;
    }
    
    if (subscribedTickers_.find(symbol) != subscribedTickers_.end()) {
        std::cout << "Already subscribed to ticker: " << symbol << std::endl;
        return;
    }

    std::string subReq = fmt::format(R"({{
        "jsonrpc": "2.0",
        "id": {},
        "method": "public/subscribe",
        "params": {{
            "channels": ["ticker.{}"]
        }}
    }})", nextId_++, symbol);

    ws_->async_write(net::buffer(subReq));
    subscribedTickers_.insert(symbol);
}

void DeribitClient::unsubscribe_ticker(const std::string& symbol) {
    if (!beast::get_lowest_layer(*ws_).is_open()) {
        std::cerr << "Cannot unsubscribe with closed WebSocket";
        return;
    }

    if (subscribedTickers_.find(symbol) == subscribedTickers_.end()) {
        std::cout << "Not subscribed to ticker: " << symbol << std::endl;
        return;
    }

    std::string unsubReq = fmt::format(R"({{
        "jsonrpc": "2.0",
        "id": {},
        "method": "public/unsubscribe",
        "params": {{
            "channels": [
                0 : "ticker.{}.agg2"
            ]
        }}
    }})", nextId_++, symbol);

    ws_->async_write(net::buffer(unsubReq));
    this->subscribedTickers_.erase(symbol);
}

void DeribitClient::subscribe_tracked(double spotPrice) {
    for (const auto& expiry: trackedExpiries_) {

        int spotThousands = spotPrice / 1000;
        for (double strike = spotThousands - 5; strike <= spotThousands + 5; strike += 1) { // For Larger coins
            std::string symbol = create_symbol(symbol_, expiry, strike);
            subscribe_ticker(symbol);
        }
    }
}

nlohmann::json DeribitClient::processPayload(const std::string& msg) {
    try {
        auto j = json::parse(msg);
        nlohmann::json normalized;

        double last_price = j["params"]["data"]["last_price"].get<double>();
        double last_quantity = j["params"]["data"]["last_quantity"].get<double>();
        double iv = j["params"]["data"]["mark_iv"].get<double>();
        std::string name = j["params"]["data"]["instrument_name"].get<std::string>();

        normalized["price"] = last_price;
        normalized["quantity"] = last_quantity;
        normalized["iv"] = iv;
        normalized["symbol"] = name;

        return normalized;

    } catch (const std::exception& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        return {};
    }
}
