#include "OrderBook.hpp"
#include <nlohmann/json>

using json = nlohmann::json;

enum 
OrderBook::OrderBook(Exchange ex, std::unique_ptr<WebSocketClient> webSocket) : ex_{ex}, webSocket_{std::move(webSocket)}, lastUpdateID_{}, bids_{}, asks_{}
{	
	webSocket->connect();
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
		if (
		json updateData = json::parse(jsonUpdate);

		if (updateData["u"] < lastUpdateID_) {
			continue;
		} else if (updateData["U"] > lastUpdateID_) {
			// TODO: Implement logic for restarting the orderbook
			break;
		}

		lastUpdateID_ = updateData["u"];
		for (auto& bid : updateData["bids"]) {
			if (!stod(bid[1])) {
				bids_.erase(stod(bid[0]));
			} else {
				bids_[stod(bid[0])] = stod(bid[1]);
			}
		}
		for (auto& ask : updateData["bids"]) {
			if (!stod(ask[1])) {
				asks_.erase(stod(ask[0]));
			} else {
				asks_[stod(ask[0])] = stod(ask[1]);
			}
		}
	}
}

void OrderBook::initOrderBook()
{
	std::string target;
	switch (ex_) {
		case Exchange::Binance:
			target = "btcusdt@depth"
	}

	// TODO: verify thread logic!!!
	webSocket_->subscribe(target);
	std::thread stream(&WebSocketClient::run, &webSocket_);

	json snapshot = json::parse(getOrderBookSnapshot(target));
	json update = json::parse(websocket_->readFromBuffer());

	while (snapshot["lastUpdateID"] < update["U"]) {
		snapshot = json::parse(getOrderBookSnapshot(target));
	}

	// First Orderbook update
	if (update["u"] > snapshot["lastUpdateID"]) {
		lastUpdateID_ = update["u"];

		for (auto& bid : update["bids"]) {
			if (!stod(bid[1])) {
				bids_.erase(stod(bid[0]));
			} else {
				bids_[stod(bid[0])] = stod(bid[1]);
			}
		}
		for (auto& ask : update["asks"]) {
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
}
