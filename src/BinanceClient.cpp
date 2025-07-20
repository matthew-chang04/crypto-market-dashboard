
#include "BinanceClient.hpp"
#include <nlomann/json>
#define BINANCE_END "wss://stream.binance.com:9443/ws/"
#define ORDERBOOK_REQ "https://api.binance.com/api/v3/depth?symbol=" + symbol + "&limit=5000"

BinanceClient::BinanceClient(std::string symbol) : WebSocketClient(BINANCE_END + symbol + "@depth") {}

void BinanceClient::onMessage(const std::string& msg) 
{
		
}

uint64_t subscribe()
{
	//TODO: implement the web socket connection	(THE RETURN VALUE WILL BE THE initial U value of the subscription)
}

void BinanceClient::getOrderBookSnapshot()
{
	//TODO: verify if this makes sense
	uint64_t initSubID = subscribe();

i	

	

}
