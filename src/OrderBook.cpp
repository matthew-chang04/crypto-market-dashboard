#include "BinanceClient.hpp"
#include "OrderBook.hpp"
#include <nlohmann/json.hpp>
#include <thread>
#include <iostream>

using json = nlohmann::json; 	

OrderBook::OrderBook() : lastUpdateID_{}, bids_{}, asks_{} {}

void OrderBook::run()
{
	
	while(true) {
		if (true) {
			if (updateThread_.joinable()) {
				updateThread_.join();
			}
			
			// requestRestart();
		}
	}
}
/*
void OrderBook::initOrderBook()
{

	// Get first orderbook snapshot
	json snapshot = json::parse(BinanceClient::getOrderBookSnapshot(target));	

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
	if (!updateThread_.joinable()) {
			updateThread_ = std::thread(&OrderBook::update, this);

	}
}
*/

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
/*
void OrderBook::writeUpdate(const std::string& update)
{
	std::lock_guard<std::mutex> lock(obMutex_);
	if (!snapshotLoaded_) {
		std::cerr << "Order book snapshot not loaded yet." << std::endl;
		return;
	}

	json data = json::parse(update);
	if (data["u"].get<uint64_t>() < lastUpdateID_) {
		std::cerr << "Received outdated update." << std::endl;
		return;
	} else if (data["U"].get<uint64_t>() > lastUpdateID_) {
		requestRestart();
		return;
	}

	lastUpdateID_ = data["u"].get<uint64_t>();
	std::vector<std::vector<std::string>> bids = data["b"].get<std::vector<std::vector<std::string>>>();
	std::vector<std::vector<std::string>> asks = data["a"].get<std::vector<std::vector<std::string>>>();

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

void OrderBook::requestRestart() {
	stop();
	initOrderBook();
}
*/