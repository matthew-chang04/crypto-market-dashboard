
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
	void read();
	void run() override;
	std::string readFromBuffer() const;

	// Reads using REST API not WS stream
	static std::string getOrderBookSnapshot() {}

private:
	std::queue<std::string> buffer_;
};

