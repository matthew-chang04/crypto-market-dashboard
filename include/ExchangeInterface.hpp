#include <string>
#include <unordered_map>


class ExchangeInterface {
public:
	virtual ~ExchangeInterface() = default;
	virtual void subscribe_orderbook() = 0;
	virtual void subscribe_ticker() = 0;
	virtual std::string normalize_symbol(const std::string& symbol) = 0;
private:
	std::string symbol_;
};
