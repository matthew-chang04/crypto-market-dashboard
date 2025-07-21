
#pragma once

#include <string>
#include <websocketpp/client.hpp>
#include "WebSocketClient.hpp"
#include "OrderBook.hpp"

class BinanceClient : public WebSocketClient
{
public:
	BinanceClient(std::string symbol) {};
	void connect() override;
	void subscribe(const std::string& target) override;
	void run();

protected:
	std::string getOrderBookSnapshot() {}
	void initOrderbook() {};

private:
	OrderBook orderbook_;
};

