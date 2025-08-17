#pragma once

#include <vector>
#include <thread>
#include <boost/asio.hpp>
#include <memory>
#include "WebSocketClient.hpp"


class ClientManager {
    public:
        ClientManager(): ioc_{}, sslCtx_{boost::asio::ssl::context::tlsv12_client}, resolver_{ioc_} {

        };
        
        template<typename Handler>
        void addFeed(std::string host, std::string port, Handler handler);
        void run(int numThreads = 2);

    private:
        boost::asio::io_context ioc_;
        boost::asio::ssl::context sslCtx_;
        tcp::resolver resolver_;
        std::vector<std::thread> pool_;
        std::vector<std::shared_ptr<WebSocketClient>> clients_;
};