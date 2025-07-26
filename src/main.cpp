#include "OrderBook.h"
#include "BinanceClient.hpp"

// DEMO usage flow for continued development/testing

int main(int argc, char *[] argv) // args: <exchange> <coin> <datatype>
{
	if (argc != 3) {
		std::err << "ERROR: Usage: ./exec <exchange> <coin> <datatype>" << std::endl;
		std::cout << "EXAMPLE: ./exec binance btc orderbook" << std::endl;
	}

	if (argv[1] == "binance") {
		auto client = std::make_unique<BinanceClient>();
		
	}
}
