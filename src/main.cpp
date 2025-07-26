#include "OrderBook.hpp"
#include "BinanceClient.hpp"

// DEMO usage flow for continued development/testing

int main(int argc, char *[] argv) // args: <exchange> <coin> <datatype>
{
	if (argc != 3) {
		std::err << "ERROR: Usage: ./exec <exchange> <coin> <datatype>" << std::endl;
		std::cout << "EXAMPLE: ./exec binance btc orderbook" << std::endl;
	}

	std::unique_ptr exchangeSocket;
	Exchange ex;
	if (argv[1] == "binance") {
		ex = Binance;
		exchangeSocket = std::make_unique<BinanceClient>();	
	} // TODO: OTHER exchages


	if (argv[3] == "orderbook") {
		OrderBook ob{ex, exchangeSocket};
		ob.initOrderBook();

		// WHERE we handle info (ie: print orders to screen)
	}

	return;
}
