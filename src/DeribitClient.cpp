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
            
            auto now = system_clock::now();
            std::time_t t = system_clock::to_time_t(now);

            std::tm tm_now;
#if defined(_WIN32) || defined(_WIN64)
            localtime_s(&tm_now, &t);
#else
            localtime_r(&t, &tm_now);
#endif
            int businessDaysCount = 0;
            while (businessDaysCount < 10) {
                
                if (tm_now.tm_wday != 0 && tm_now.tm_wday != 6) { // 0=Sunday,6=Saturday
                    std::cout << format_date(tm_now) << std::endl;
                    ++businessDaysCount;
                }

                t += 24*60*60; // add one day in seconds
#if defined(_WIN32) || defined(_WIN64)
                localtime_s(&tm_now, &t);
#else
                localtime_r(&t, &tm_now);
#endif
            }
        }


std::string DeribitClient::normalize_symbol(const std::string& symbol) {
    std::string normalized;
    for (char ch : symbol) {
        normalized += std::toupper(ch);
    }
    return normalized; 
}

std::string DeribitClient::format_date(std::tm tm) {

    // Format as DDMMMYY
    std::ostringstream oss;
    oss << std::setw(2) << std::setfill('0') << tm.tm_mday; // DD

    static const char* months[] = {
        "JAN","FEB","MAR","APR","MAY","JUN",
        "JUL","AUG","SEP","OCT","NOV","DEC"
    };
    oss << months[tm.tm_mon]; // MMM

    oss << std::setw(2) << (tm.tm_year % 100); // YY

    return oss.str();
}

std::string DeribitClient::create_symbol(const std::string& base, const std::string& expiry, double strike) {
    std::string symbol = fmt::format("{}-{}-{}-C", base, expiry, static_cast<int>(strike));
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
            "channels": ["ticker.{}"]
        }}
    }})", nextId_++, symbol);

    ws_->async_write(net::buffer(unsubReq));
    subscribedTickers_.erase(symbol);
}

void DeribitClient::ticker_handler(const std::string& msg) {
    try {
        auto j = json::parse(msg);

        if (j.contains("method") && j["method"] == "subscription") {
            auto params = j["params"];
            std::string channel = params["channel"];
            auto data = params["data"];

            if (channel.rfind("ticker.", 0) == 0) { // starts with "ticker."
                std::string symbol = channel.substr(7); // remove "ticker."

                double price = data["last_price"].get<double>();
                double quantity = data["last_quantity"].get<double>();
                auto timestamp = std::chrono::system_clock::now();

                SpotTick tick{price, quantity, timestamp};
                dataManager_.addSpotTick(tick);
            }
        } else if (j.contains("error")) {
            std::cerr << "Error from Deribit: " << j["error"]["message"] << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Failed to parse ticker message: " << e.what() << std::endl;
    }
}