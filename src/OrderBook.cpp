#include "OrderBook.cpp"
#include <nlohmann/json>

using nlohmann::json = json;

OrderBook::OrderBook() : lastUpdateID{}, bids{}, asks{} {}

OrderBook::populateSnapshot(const std::string& response)
{
	json orderData json::parse(response);
	lastUpdateID = orderData["lastUpdateID"];

	for (auto& bid : orderData["bids"]) {
		bids.insert({stod(bid[0]), stod(bid[1])});
	}
	for (auto& ask : orderData["asks"]) {
		asks.insert({stod(ask[0]), stod(ask[1])});
	}
}

OrderBook::update(const std::string& jsonUpdate)
{
	json updateData = json::parse(jsonUpdate);
	lastUpdateID = updateData["u"];

	for (auto& bid : updateData["bids"]) {
		if (!stod(bid[1])) {
			bids.erase(stod(bid[0]));
		} else {
			bids[stod(bid[0])] = stod(bid[1]);
		}
	}
}
:
