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

	try {
		webSocket_->connect();
		webSocket_->subscribe(target);
		webSocket_->run();
	} catch (const std::exception& e) {
		std::cerr << "Error initializing WebSocket connection: " << e.what() << std::endl;
		throw;
	}

	json snapshot = json::parse(BinanceClient::getOrderBookSnapshot(target));
	std::cout << "Order Book Snapshot: " << snapshot.dump(4) << std::endl;
 
	std::optional<std::string> rawUpdate;
	do {
		rawUpdate = webSocket_->readFromBuffer();
	} while (!rawUpdate.has_value());
	json update = json::parse(rawUpdate.value());
	std::cout << "Order Book Update: " << update.dump(4) << std::endl;
	
/*	if (update["code"].get<uint64_t>() < 0) {
		std::cerr << "Error making HTTP request: " << update["msg"].get<std::string>() << std::endl;
		throw;
	}

*/
	json data = update["data"];
	while (snapshot["lastUpdateId"].get<uint64_t>() < data["U"].get<uint64_t>()) {
		snapshot = json::parse(BinanceClient::getOrderBookSnapshot(target));
		std::cout << "Order Book Snapshot: " << snapshot.dump(4) << std::endl;
	}

	// First Orderbook update
	if (data["u"].get<uint64_t>() > snapshot["lastUpdateId"].get<uint64_t>()) {
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

	populateSnapshot(snapshot);
	for (auto it = asks_.rbegin(); it != asks_.rend() && std::distance(asks_.rbegin(), it) < 5; ++it) {
		std::cout << it->first << " : " << it->second << "\n";
	}
		
	std::thread orderParser(&OrderBook::update, this);
	orderParser.detach();
}

void OrderBook::stop()
{
	webSocket_->stop();
	stopped_ = true;
	bids_.clear();
	asks_.clear();
	lastUpdateID_ = -1;
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
}

void OrderBook::update()
{
	while (true) {
		if (stopped_) return;

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

		lastUpdateID_ = updateData["u"].get<uint64_t>();
		std::vector<std::vector<double>> bids = updateData["b"].get<std::vector<std::vector<double>>>();
		std::vector<std::vector<double>> asks = updateData["a"].get<std::vector<std::vector<double>>>();

		for (std::vector<double>& bid : bids) {
			double price = bid[0];
			double quantity = bid[1];

			if (!quantity) {
				bids_.erase(price);
			} else {
				bids_[price] = quantity; 
			}
		}
		for (std::vector<double> ask : asks) {
			double price = ask[0];
			double quantity = ask[1];

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
