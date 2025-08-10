#include "BinanceClient.hpp"
#include "OrderBook.hpp"
#include <nlohmann/json.hpp>
#include <thread>
#include <iostream>

using json = nlohmann::json;

OrderBook::OrderBook(Exchange ex, std::unique_ptr<WebSocketClient> webSocket) : ex_{ex}, webSocket_{std::move(webSocket)}, lastUpdateID_{}, bids_{}, asks_{}, stopped_{false}
{	

}

void OrderBook::initOrderBook()
{
	std::string target;
	switch (ex_) {
		case Exchange::Binance:
			target = "btcusdt@depth";
			break;
		case Exchange::Kraken:
			break;
		case Exchange::Coinbase:
			break;
	}
	// Set up WebSocket
	try {
		webSocket_->connect();
		webSocket_->subscribe(target);
		webSocket_->run();
	} catch (const std::exception& e) {
		std::cerr << "Error initializing WebSocket connection: " << e.what() << std::endl;
		throw;
	}

	// Get first orderbook snapshot
	json snapshot = json::parse(BinanceClient::getOrderBookSnapshot(target));
 
	std::optional<std::string> rawUpdate;
	do {
		rawUpdate = webSocket_->readFromBuffer();
	} while (!rawUpdate.has_value());

	json update = json::parse(rawUpdate.value());
	
	// Make sure update is actually newer
	json data = update["data"];
	while (snapshot["lastUpdateId"].get<uint64_t>() < data["U"].get<uint64_t>()) {
		snapshot = json::parse(BinanceClient::getOrderBookSnapshot(target));
	}

	// First Orderbook update
	if (data["u"].get<uint64_t>() > snapshot["lastUpdateId"].get<uint64_t>()) {
		lastUpdateID_ = data["u"].get<uint64_t>();

		std::vector<std::vector<std::string>> bids = data["b"].get<std::vector<std::vector<std::string>>>();
		std::vector<std::vector<std::string>> asks = data["a"].get<std::vector<std::vector<std::string>>>();

		std::cout << data["a"].dump(4) << std::endl;

		for (const std::vector<std::string>& bid : bids) {
			double price = stod(bid[0]);
			double quantity = stod(bid[1]);

			if (!quantity) {
				bids_.erase(price);
			} else {
				bids_[price] = quantity;
			}
		}
		for (const std::vector<std::string>& ask : asks) {

			double price = stod(ask[0]);
			double quantity = stod(ask[1]);

			if (!quantity) { 
				asks_.erase(price);
			} else {
				asks_[price] = quantity;
			}
		}
	}

	populateSnapshot(snapshot);
	stopped_ = false;
	std::thread orderParser(&OrderBook::update, this);
	orderParser.detach();
}

void OrderBook::stop()
{
	std::lock_guard<std::mutex> lock(obMutex_);
	webSocket_->stop();
	bids_.clear();
	asks_.clear();
	lastUpdateID_ = -1;
	snapshotLoaded_ = false;
	stopped_ = true;
}

void OrderBook::populateSnapshot(const json& orderData)
{
	lastUpdateID_ = orderData["lastUpdateId"].get<uint64_t>();

	std::vector<std::vector<std::string>> bids = orderData["bids"].get<std::vector<std::vector<std::string>>>();
	std::vector<std::vector<std::string>> asks = orderData["asks"].get<std::vector<std::vector<std::string>>>();

	for (const std::vector<std::string>& bid : bids) {
		double price = stod(bid[0]);
		double quantity = stod(bid[1]);
		bids_.insert({price, quantity});
	}
	for (const std::vector<std::string>& ask : asks) {
		double price = stod(ask[0]);
		double quantity = stod(ask[1]);
		asks_.insert({price, quantity});
	}
	snapshotLoaded_ = true;
}

void OrderBook::update()
{
	while (true) {
		if (webSocket_->isInterrupted()) {
			stop();
			initOrderBook();
			webSocket_->setInterrupted(false);
			return;
		}

		std::optional<std::string> rawUpdate = webSocket_->readFromBuffer();
		json updateData;
		if (!rawUpdate.has_value()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			continue;
		}
		updateData = json::parse(rawUpdate.value());

		updateData = updateData["data"];
		if (updateData["u"].get<uint64_t>() < lastUpdateID_) {
			continue;
		} else if (updateData["U"].get<uint64_t>() > lastUpdateID_) { // Need to reset orderbook
			stop();
			initOrderBook(); 
			break;
		}
		
		std::lock_guard<std::mutex> lock(obMutex_);

		lastUpdateID_ = updateData["u"].get<uint64_t>();
		std::vector<std::vector<std::string>> bids = updateData["b"].get<std::vector<std::vector<std::string>>>();
		std::vector<std::vector<std::string>> asks = updateData["a"].get<std::vector<std::vector<std::string>>>();

		for (std::vector<std::string>& bid : bids) {
			double price = stod(bid[0]);
			double quantity = stod(bid[1]);

			if (!quantity) {
				bids_.erase(price);
			} else {
				bids_[price] = quantity; 
			}
		}
		for (std::vector<std::string> ask : asks) {
			double price = stod(ask[0]);
			double quantity = stod(ask[1]);

			if (!quantity) {
				asks_.erase(price);
			} else {
				asks_[price] = quantity;
			}
		}
	}
}

void OrderBook::testRun()
{
	if (!snapshotLoaded_) {
		std::cout << "Waiting for order book snapshot..." << std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(5));
		return;
	}

	std::lock_guard<std::mutex> lock(obMutex_);
	// std::cout << "\033[2J\033[1;1H"; // Clear screen

	std::cout << "     Order Book\n";
	std::cout << "-------------------\n";
	std::cout << "     Asks\n";
	for (auto it = asks_.rbegin(); it != asks_.rend() && std::distance(asks_.rbegin(), it) < 5; ++it) {
		std::cout << it->first << " : " << it->second << "\n";
		}

	std::cout << "-------------------\n";
	std::cout << "     Bids\n";
	for (auto it = bids_.begin(); it != bids_.end() && std::distance(bids_.begin(), it) < 5; ++it) {			
		std::cout << it->first << " : " << it->second << "\n";

	}

	std::cout.flush();
}

void OrderBook::testLoop()
{
	while(true) {
		testRun();
	}
}
