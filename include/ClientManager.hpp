#pragma once

#include <vector>
#include <thread>
#include <boost/asio.hpp>
#include <memory>
#include "WebSocketClient.hpp"

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
        };
        
        template<typename Handler>
        void addFeed(std::string host, std::string port, std::string target, Handler handler);
        void run(int numThreads = 2);
		void startFeeds();

    private:
        boost::asio::io_context ioc_;
        boost::asio::ssl::context sslCtx_;
        tcp::resolver resolver_;
        std::vector<std::thread> pool_;
        std::vector<std::shared_ptr<WebSocketClient>> clients_;
        std::mutex mapMutex_;
        std::unordered_map<std::shared_ptr<WebSocketClient>, std::queue<std::string>> clientMap_;
};
