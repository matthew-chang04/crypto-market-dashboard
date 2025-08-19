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

gemplate<typename Handler>
void ClientManager::addFeed(std::string host, std::string port, Handler handler) {
    auto client = std::make_shared<WebSocketClient>(ioc_, sslCtx_);
    client->setHost(std::move(host));
    client->setPort(std::move(port));
    client->setHandler(handler);
    clients_.push_back(client);
}

void ClientManager::startFeeds() {
	for (std::vector<std::shared_ptr<WebSocketClient>> client : clients_) {
		client->start();
		client->subscribe(
		
	}	
}


