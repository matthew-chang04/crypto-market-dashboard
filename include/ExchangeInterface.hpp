#include <string>
#include <unordered_map>


class ExchangeInterface {
public:
	virtual ~ExchangeInterface() = default;
	virtual void subscribe_orderbook(const std::string& symbol) = 0;
	virtual void subscribe_ticker(const std::string& symbol) = 0;
	virtual std::string normalize_symbol(const std::string& symbol) = 0;
};
