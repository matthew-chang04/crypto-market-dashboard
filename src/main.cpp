#include "CoinbaseClient.hpp"
#include "ClientManager.hpp"
#include <iostream>
#include <err.h>
#include <utility>
#include <memory>

  // DEMO usage flow for continued development/testing
  
int main(int argc, char * argv[]) // args: <exchange> <coin> <datatype>
{
	
	ClientManager clientManager{"BTC"};

	std::cout << "add coinbase" << std::endl;
	clientManager.addFeed("Coinbase", CoinbaseClient::HOST, CoinbaseClient::PORT, "/");
	
	std::cout << "add deribit" << std::endl;
	clientManager.addFeed("Deribit", DeribitClient::HOST, DeribitClient::PORT, "/ws/api/v2");

	clientManager.run(3);
	clientManager.startFeeds();

	while (true) {
		clientManager.sendData();
	}	
	return 0;
}
