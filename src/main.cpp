#include "BinanceClient.hpp"
#include "ClientManager.hpp"
#include <iostream>
#include <err.h>
#include <utility>
#include <memory>

// DEMO usage flow for continued development/testing
  
int main(int argc, char * argv[]) // args: <exchange> <coin> <datatype>
{
	
	ClientManager clientManager;
	MarketDataManager dataManager;

	std::shared_ptr<MarketDataManager> dmPointer = std::make_shared<MarketDataManager>(dmPointer);

	clientManager.addFeed(CoinbaseClient::HOST, CoinbaseClient::PORT, CoinbaseClient::tickerUrl, std::copy(dmPointer));
	clientManager.addFeed(DeribitClient::HOST, DeribitClient::PORT, DeribitClient::impliedVolUrl, std::copy(dmPointer));

	clientManager.run(2);
	clientManager.startFeeds();

	return 0;
}
