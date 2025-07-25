
#pragma once

#include <string>
#include <websocketpp/client.hpp>
#include "WebSocketClient.hpp"

class BinanceClient : public WebSocketClient
{
public:
	BinanceClient() {};
	void handleMessage(std::string payload);
	void connect() override;
	void subscribe(const std::string& target) override;
	void run() override;
	std::string readFromBuffer();

protected:
	static std::string getOrderBookSnapshot() {}

private:
	std::queue<std::string> buffer;
};

