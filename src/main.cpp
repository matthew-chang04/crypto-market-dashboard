#include "BinanceClient.hpp"
#include "OrderBook.hpp"
#include "ClientManager.hpp"
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

	ClientManager clientManager;

	if (!strcmp("binance", argv[1])) {
		

		if (!strcmp("orderbook", argv[3])) {
			clientManager.addFeed(BinanceClient::HOST, BinanceClient::PORT, std::string(argv[2]) + "@depth", [](const std::string& message) {
				std::cout << "Received message: " << message << std::endl;
			});
			clientManager.run(2);
			clientManager.startFeeds();
		} else {
			std::cout << "Issue with string recognition (orderbook)" << std::endl;
		}
	} else {
		std::cout << "Issue with String Recognition (binance)" << std::endl;
	}
	 // TODO: OTHER exchages

	return 0;
}
