#include "WebSocketClient.hpp"
#include "ExchangeInterface.hpp"
#include <string>

class CoinbaseClient: public WebSocketClient, public ExchangeInterface {
    public:
        const static std::string HOST;
        const static std::string PORT;

        CoinbaseClient(net::io_context& ioc, net::ssl::context& sslCtx, tcp::resolver& resolver, std::string target, std::string symbol)
            : WebSocketClient(ioc, sslCtx, resolver, target, symbol) {}

        std::string normalize_symbol(const std::string& symbol) override;
        void subscribe_orderbook() override; 
        void subscribe_ticker() override;
        nlohmann::json parsePayload(const std::string& msg) override;
};

