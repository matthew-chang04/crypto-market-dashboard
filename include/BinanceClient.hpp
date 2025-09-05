#pragma once
#include <optional>
#include <condition_variable>
#include <string>
#include <queue>
#include <boost/beast/core.hpp>
#include <optional>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/ssl.hpp>
#include "WebSocketClient.hpp"
#include "ExchangeInterface.hpp"

namespace beast = boost::beast;

class BinanceClient : public WebSocketClient, 
{
public:
	const static  std::string HOST;
	const static std::string PORT;

	BinanceClient(net::io_context& ioc, net::ssl::context& sslCtx, tcp::resolver& resolver, std::string target, std::string symbol)
		: WebSocketClient(ioc, sslCtx, resolver, target, symbol) {
			setHost(HOST);
			setPort(PORT);
		}

	void subscribe_orderbook(const std::string& symbol) override;
	void subscribe_ticker(const std::string& symbol) override;
	std::string normalize_symbol(const std::string& symbol) override;


	// Reads using REST API not WS stream
	static std::string getOrderBookSnapshot(const std::string& target);

};

