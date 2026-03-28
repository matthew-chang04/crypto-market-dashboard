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

	clientManager.addFeed("Coinbase", CoinbaseClient::HOST, CoinbaseClient::PORT, "BTC-USD");
	clientManager.addFeed("Deribit", DeribitClient::HOST, DeribitClient::PORT, "BTC-USD");

	clientManager.run(2);
	clientManager.startFeeds();

	

	return 0;
}
