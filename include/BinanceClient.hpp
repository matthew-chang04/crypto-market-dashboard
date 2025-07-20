
#pragma once

#include <string>
#include <websocketpp/client.hpp>
#include "WebSocketClient.hpp"

class BinanceClient : public WebSocketClient
{
public:
	BinanceClient(std::string symbol) {};

protected:
	void on_message(const std::string& msg) override;
	void subscribe() override;
}

