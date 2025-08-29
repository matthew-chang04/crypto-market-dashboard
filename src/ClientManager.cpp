#include "ClientManager.hpp"
#include "WebSocketClient.hpp"
#include <memory>
#include <iostream>
#include <stdexcept>

namespace beast = boost::beast;   
namespace http = beast::http;          
namespace websocket = beast::websocket;
namespace net = boost::asio;           
using tcp = boost::asio::ip::tcp;     

void ClientManager::run(int numThreads) {
    for (int i = 0; i < numThreads; ++i) {
        pool_.emplace_back([this]() {
            try {
                ioc_.run();
            } catch (const std::exception& e) {
                std::cerr << "Error in thread: " << e.what() << std::endl;
            }
        });
    }
}

template<typename Handler>
void ClientManager::addFeed(std::string host, std::string port, std::string target, Handler handler) {
    auto client = std::make_shared<WebSocketClient>(ioc_, sslCtx_);
    client->setHost(std::move(host));
    client->setPort(std::move(port));
    client->setTarget(target); 
    client->setHandler(handler);
    clients_.push_back(client);
}

void ClientManager::startFeeds() {
	for (auto client : clients_) {
		client->start();
		client->subscribe(client->getTarget());
	}
}

void ClientManager::stopFeeds() {
    for (auto client : clients_) {
        client->stop();
    }
}

void ClientManager::createOrderBook(const std::string& symbol) {
    // 0. Set up orderbook (need to get snapshot from binance )
    
    // 1. make connections to binance, kraken, coinbase
    /*
        handle functions will just read to json?? or maybe string?
        - the client manager will deal with sending websocket responses (already normalized) to the orderbook buffer.
        
    */
    // 2. read from these to their respective queues

    // 3. 
}


