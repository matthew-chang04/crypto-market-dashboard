#include <string>
#include <unordered_map>
#include <function>


enum class Exchange {
	Binance,
	Coinbase,
	Kraken
};

struct ExchangeInfo {
	std::string host;
	std::string port;
	std::unordered_map<Exchange, std::function<std::string(const std::string& symbol)> subscribeBuilders;
};


