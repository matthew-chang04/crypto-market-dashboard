#include "BinanceClient.hpp"
#include "ClientManager.hpp"
#include <iostream>
#include <err.h>
#include <utility>


// DEMO usage flow for continued development/testing

int main(int argc, char * argv[]) // args: <exchange> <coin> <datatype>
{
	
	ClientManager clientManager;

	clientManager.addFeed(BinanceClient::HOST, BinanceClient::PORT, std::string(argv[2]) + "@depth", [](const std::string& message) {
		std::cout << "Received message: " << message << std::endl;
	});

	clientManager.run(2);
	clientManager.startFeeds();

	return 0;
}
