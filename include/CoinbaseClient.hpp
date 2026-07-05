#include "WebSocketClient.hpp"
#include <string>

class CoinbaseClient: public WebSocketClient {
    public:
        const static std::string HOST;
        const static std::string PORT;

        CoinbaseClient(net::io_context& ioc, net::ssl::context& sslCtx, tcp::resolver& resolver, std::string target, std::string symbol)
            : WebSocketClient(ioc, sslCtx, resolver, target, symbol) {
                setHost(HOST);
                setPort(PORT);
            }

        const std::string& normalizeSymbol(const std::string& symbol) override;
        std::optional<MarketEvent> parsePayload(const std::string& msg) override;
        const nlohmann::json& buildRequestMsg(const std::string& action, const std::string& product) override;

};

