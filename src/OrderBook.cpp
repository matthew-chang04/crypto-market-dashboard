#include "OrderBook.hpp"
#include <nlohmann/json>

using json = nlohmann::json;

enum 
OrderBook::OrderBook(Exchange ex, std::unique_ptr<WebSocketClient> webSocket) : ex_{ex}, webSocket_{std::move(webSocket)}, lastUpdateID_{}, bids_{}, asks_{}
{	
	webSocket->connect();
}

void OrderBook::populateSnapshot(const std::string& response)
{
	json orderData = json::parse(response);
	lastUpdateID_ = orderData["lastUpdateID"];

	for (auto& bid : orderData["bids"]) {
		bids.insert({stod(bid[0]), stod(bid[1])});
	}
	for (auto& ask : orderData["asks"]) {
		asks.insert({stod(ask[0]), stod(ask[1])});
	}
}

void OrderBook::update(const std::string& jsonUpdate)
{
	json updateData = json::parse(jsonUpdate);
	lastUpdateID_ = updateData["u"];

	for (auto& bid : updateData["bids"]) {
		if (!stod(bid[1])) {
			bids_.erase(stod(bid[0]));
		} else {
			bids_[stod(bid[0])] = stod(bid[1]);
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

	webSocket_->subscribe(target);
	std::thread stream(webSocket_->run);

	




	
}
