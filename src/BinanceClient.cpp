
#include "BinanceClient.hpp"

#define BINANCE_END "wss://stream.binance.com:9443"

BinanceClient::BinanceClient(std::string symbol) : WebSocketClient(BINANCE_END + symbol + "@trade") {}



