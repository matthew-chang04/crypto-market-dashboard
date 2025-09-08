#pragma once

#include <vector>
#include <thread>
#include <boost/asio.hpp>
#include <memory>
#include "WebSocketClient.hpp"
#include "OrderBook.hpp"
#include "DataManager.hpp"
#include "DeribitClient.hpp"

namespace beast = boost::beast;   
namespace http = beast::http;          
namespace websocket = beast::websocket;
namespace net = boost::asio;           
using tcp = boost::asio::ip::tcp;     

class ClientManager {
    public:
        ClientManager(): ioc_{}, sslCtx_{boost::asio::ssl::context::tlsv12_client}, resolver_{ioc_} {
            sslCtx_.set_options(boost::asio::ssl::context::default_workarounds);
            sslCtx_.set_verify_mode(boost::asio::ssl::verify_peer);
        }
        
        void addFeed(std::string host, std::string port, std::string target, std::function<void(const std::string&)> handler);
        void addOptionFeed(std::string host, std::string port, std::string target, MarketDataManager& dataManager);
        void run(int numThreads = 2);
		void startFeeds();
        void stopFeeds();

        void updateATM(double spot, int strikeRange = 5);
        void createOrderBook(const std::string& symbol);

    private:
        boost::asio::io_context ioc_;
        boost::asio::ssl::context sslCtx_;
        tcp::resolver resolver_;
        std::vector<std::thread> pool_;
        
        std::vector<std::shared_ptr<WebSocketClient>> clients_;
        std::shared_ptr<DeribitClient> optionsClient_;

        MarketDataManager dataManager_;
};
