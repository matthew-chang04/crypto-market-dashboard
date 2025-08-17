#include "BinanceClient.hpp"
#include "OrderBook.hpp"
#include <iostream>
#include <err.h>
#include <utility>


// DEMO usage flow for continued development/testing

int main(int argc, char * argv[]) // args: <exchange> <coin> <datatype>
{
	if (argc != 4) {
		std::cout << argc << std::endl;
		std::cerr << "ERROR: Usage: ./Exec <exchange> <coin> <datatype>" << std::endl;
		std::cout << "EXAMPLE: ./Exec binance btc orderbook" << std::endl;
		return 0;
	}

	std::unique_ptr<WebSocketClient> exchangeSocket;
	Exchange ex;
	if (!strcmp("binance", argv[1])) {
		ex = Exchange::Binance;
		exchangeSocket = std::make_unique<BinanceClient>();	

		if (!strcmp("orderbook", argv[3])) {
			OrderBook ob{ex, std::move(exchangeSocket)};
			ob.initOrderBook();
			ob.testLoop();
		} else {
			std::cout << "Issue with string recognition (orderbook)" << std::endl;
		}
	} else {
		std::cout << "Issue with String Recognition (binance)" << std::endl;
	}
	 // TODO: OTHER exchages

	return 0;
}
