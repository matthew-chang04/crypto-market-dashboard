#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <iostream>
#include <format>
#include <websocketpp/client.hpp>
#include "WebSocketClient.hpp"

namespace beast = boost::beast;

class BinanceClient : public WebSocketClient
{
public:
	BinanceClient() {};
	void connect() override;
	void subscribe(const std::string& target) override;
	void read();
	void run() override;
	std::string readFromBuffer() const;
	void stop() {}

	// Reads using REST API not WS stream
	static std::string getOrderBookSnapshot() {}

private:
	std::queue<std::string> buffer_;
	beast::flat_buffer readDump_;
};

