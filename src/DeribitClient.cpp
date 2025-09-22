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

            std::vector<std::string> trackedExpiries;
        int businessDaysCount = 0;

        while (businessDaysCount < 10) {
            // Get weekday in UTC
            weekday wd{day};
            if (wd != Sunday && wd != Saturday) {
                trackedExpiries.push_back(format_date(day));
                ++businessDaysCount;
            }
            day += days{1};
        }
        }

        


std::string DeribitClient::normalize_symbol(const std::string& symbol) {
    std::string normalized;
    for (char ch : symbol) {
        normalized += std::toupper(ch);
    }
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
    std::string symbol = fmt::format("{}-{}-{}-C", base, expiry, static_cast<int>(strike));
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
    subscribedTickers_.erase(symbol);
}

void DeribitClient::ticker_handler(const std::string& msg) {
    try {
        auto j = json::parse(msg);

        /*
        JSON Format for a ticker message from deribit:
        
        
        
        }*/
      // https://test.deribit.com/api_console/?method=%2Fprivate%2Fsubscribe&channels=ticker.%7Binstrument_name%7D.%7Binterval%7D
        // THIS is the testing suite for the websocket to deribit, play ariund to see the json format and make sure the request is correct

        double last_price = j["params"]["data"]["last_price"].get<double>();
        double last_quantity = j["params"]["data"]["last_quantity"].get<double>();
        double iv = j["params"]["data"]["mark_iv"].get<double>();
        std::string name = j["params"]["data"]["instrument_name"].get<std::string>();
        dataManager_.addOptionTick(name, OptionTick{last_price, last_quantity, iv, std::chrono::system_clock::now()});

    } catch (const std::exception& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        return;
    }
}