#include "ClientManager.hpp"
#include "CoinbaseClient.hpp"
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
                std::cout << "Running Thread" << std::endl;
                ioc_.run();
            } catch (const std::exception& e) {
                std::cerr << "Error in thread: " << e.what() << std::endl;
            }
        });
    }
}

void ClientManager::addFeed(const std::string& exchange, std::string host, std::string port, std::string target) {

    std::shared_ptr<WebSocketClient> client;
    if (strcmp(exchange.c_str(), "Coinbase") == 0) {
        client = std::make_shared<CoinbaseClient>(ioc_, sslCtx_, resolver_, target, asset_);
    } else if (strcmp(exchange.c_str(), "Deribit") == 0) {
        client = std::make_shared<DeribitClient>(ioc_, sslCtx_, resolver_, target, asset_);
    }
     
    clients_.push_back(client);
}

void ClientManager::addOptionFeed(std::string host, std::string port, std::string target) {
    optionsClient_ = std::make_shared<DeribitClient>(ioc_, sslCtx_, resolver_, target, asset_);
}

void ClientManager::startFeeds() {
	for (auto client : clients_) {
        std::cout << "starting next client" << std::endl;
		client->start();
        while (client->isInterrupted()) {
            continue;
        }
        
        std::cout << "Subscribing to asset " << asset_ << std::endl;
		client->subscribe(asset_, "ticker"); // default to ticker for now until orderbook logic is built
	}
}

void ClientManager::stopFeeds() {
    for (auto client : clients_) {
        client->stop();
    }

    for (auto& thread : pool_) {
        thread.join();
    }

    workGuard_.reset();
}

void ClientManager::sendData() {
    for (std::shared_ptr<WebSocketClient> client : clients_) {
        while(client->hasMessages()) {
            nlohmann::json msg = client->getNextMessage();
            dataManager_->processMessage(msg);
        }
    }
}

void ClientManager::updateATM(double spot, int strikeRange) {
    double atmStrike = std::round(spot / strikeRange) * strikeRange;

    std::vector<double> strikes;
    for (int i = -strikeRange; i <= strikeRange ; ++i) {
        double strike = atmStrike + i * strikeRange;
        if (strike > 0) { // avoid negative/zero strikes
            strikes.push_back(strike);
        }
    }

    std::vector<std::string> newSubs;
    for (std::string expiry : optionsClient_->trackedExpiries_) {
        for (double strike : strikes) {
            std::string symbol = optionsClient_->create_symbol(optionsClient_->normalize_symbol(asset_), expiry, strike);
            newSubs.push_back(symbol);
            optionsClient_->subscribe_ticker(symbol);
        }
    }

    // removing old subs
    for (const auto& sub: optionsClient_->subscribedTickers_) {
        if (std::find(newSubs.begin(), newSubs.end(), sub) == newSubs.end()) {
            optionsClient_->unsubscribe_ticker(sub);
        }
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

void ClientManager::tick() {

}
