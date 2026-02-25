#include "WebSocketClient.hpp"
#include "ExchangeInterface.hpp"
#include <string>

class KrakenClient: public WebSocketClient {
public:
    const static std::string HOST;
    const static std::string PORT;

    KrakenClient(net::io_context& ioc, net::ssl::context& sslCtx, tcp::resolver& resolver, std::string target, std::string symbol)
        : WebSocketClient(ioc, sslCtx, resolver, target, symbol) {
            setHost(HOST);
            setPort(PORT);
        }
    
    std::string normalize_symbol(const std::string& symbol) override;
    void subscribe_orderbook() override;
    void subscribe_ticker() override;

    std::string orderbookSetup();

    
};