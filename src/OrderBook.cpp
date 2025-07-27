#include "OrderBook.hpp"
#include <nlohmann/json>

using json = nlohmann::json;

OrderBook::OrderBook(Exchange ex, std::unique_ptr<WebSocketClient> webSocket) : ex_{ex}, webSocket_{std::move(webSocket)}, lastUpdateID_{}, bids_{}, bidsUpdated_{}, asks_{}, asksUpdated_{}, stopped_{false}
{	

}

void OrderBook::initOrderBook()
{
	std::string target;
	switch (ex_) {
		case Exchange::Binance:
			target = "btcusdt@depth"
	}
	webSocket_->connect();
	webSocket_->subscribe(target);
	webSocket_->run();

	// POSSIBLE SOURCE OF ERROR: Reading from the websocket Buffer
	json snapshot = json::parse(getOrderBookSnapshot(target));
	json update = json::parse(websocket_->readFromBuffer());

	while (snapshot["lastUpdateID"] < update["U"]) {
		snapshot = json::parse(getOrderBookSnapshot(target));
	}

	// First Orderbook update
	if (update["u"] > snapshot["lastUpdateID"]) {
		lastUpdateID_ = update["u"];

		for (auto& bid : update["bids"]) {
			bidsUpdated_.push_back(bid[0]);

			if (!stod(bid[1])) {
				bids_.erase(stod(bid[0]));
			} else {
				bids_[stod(bid[0])] = stod(bid[1]);
			}
		}
		for (auto& ask : update["asks"]) {
			asksUpdated_.push_back(ask[0]);

			if (!stod(ask[1])) {
				asks_.erase(stod(ask[0]));
			} else {
				asks_[stod(ask[0])] = stod(ask[1]);
			}
		}
	}
	// TODO: verify thread logic!!!
	populateSnapshot(snapshot);
	std::thread orderParser(&OrderBook::update, this);
	orderParser.detatch();
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

	lastUpdateID_ = orderData["lastUpdateID"];

	for (auto& bid : orderData["bids"]) {
		bids.insert({stod(bid[0]), stod(bid[1])});
	}
	for (auto& ask : orderData["asks"]) {
		asks.insert({stod(ask[0]), stod(ask[1])});
	}
}

void OrderBook::update()
{
	while (true) {
		if (stopped_) return;

		json updateData;
		if (webSocket_->buffer_.isEmpty()) {
			continue;
		} else {
			updateData = webSocket_.readFromBuffer();
		}

		if (updateData["u"] < lastUpdateID_) {
			continue;
		} else if (updateData["U"] > lastUpdateID_) { // Need to reset orderbook
			stop();
			initOrderBook(); 
			break;
		}

		lastUpdateID_ = updateData["u"];
		for (auto& bid : updateData["bids"]) {
			bidsUpdated_.push_back(bid[0]);

			if (!stod(bid[1])) {
				bids_.erase(stod(bid[0]));
			} else {
				bids_[stod(bid[0])] = stod(bid[1]);
			}
		}
		for (auto& ask : updateData["asks"]) {
			asksUpdated_.push_back(ask[0]);

			if (!stod(ask[1])) {
				asks_.erase(stod(ask[0]));
			} else {
				asks_[stod(ask[0])] = stod(ask[1]);
			}
		}
	}
}

void OrderBook::testRun()
{
	std::lock_guard<std::mutex> lock(obMutex_);
	std::cout << "\033[2J\033[1;1H"; // Clear screen

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
