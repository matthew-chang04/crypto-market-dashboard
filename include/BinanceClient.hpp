
#pragma once

#include <string>
#include <websocketpp/client.hpp>
#include "WebSocketClient.hpp"
#include "OrderBook.hpp"

class BinanceClient : public WebSocketClient
{
public:
	BinanceClient(std::string symbol) {};

protected:
	void onMessage(const std::string& msg) override;
	uint64_t subscribe() override;
	void getOrderBookSnapshot() {}

private:
	OrderBook orderbook;
}

