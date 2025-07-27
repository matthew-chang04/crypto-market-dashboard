#include "BinanceClient.hpp"
#include "OrderBook.hpp"
#include <iostream>
#include <err.h>
#include <utility>

// DEMO usage flow for continued development/testing

int main(int argc, char * argv[]) // args: <exchange> <coin> <datatype>
{
	if (argc != 3) {
		std::cerr << "ERROR: Usage: ./exec <exchange> <coin> <datatype>" << std::endl;
		std::cout << "EXAMPLE: ./exec binance btc orderbook" << std::endl;
	}

	std::unique_ptr<WebSocketClient> exchangeSocket;
	Exchange ex;
	if (argv[1] == "binance") {
		ex = Exchange::Binance;
		exchangeSocket = std::make_unique<BinanceClient>();	

		if (argv[3] == "orderbook") {
			OrderBook ob{ex, std::move(exchangeSocket)};
			ob.initOrderBook();
			ob.testLoop();
		}
	}
	 // TODO: OTHER exchages

	return;
}
