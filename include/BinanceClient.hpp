
#pragma once

#include <string>
#include <websocketpp/client.hpp>
#include "WebSocketClient.hpp"

class BinanceClient : public WebSocketClient
{
public:
	BinanceClient() {};
	void connect() override;
	void subscribe(const std::string& target) override;
	void run() override;

protected:
	std::string getOrderBookSnapshot() {}
	void initOrderbook() {};

private:
	std::vector<std::string> orderBuffer;
};

