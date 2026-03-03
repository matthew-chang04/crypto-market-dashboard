#include "WebSocketClient.hpp"
#include <string>

class KrakenClient: public WebSocketClient {
public:
    const static std::string HOST;
    const static std::string PORT;

    KrakenClient(net::io_context& ioc, net::ssl::context& sslCtx, tcp::resolver& resolver, std::string target, std::vector<std::string> symbols)
        : WebSocketClient(ioc, sslCtx, resolver, target, symbols) {
            setHost(HOST);
            setPort(PORT);
        }
    
    std::string normalize_symbol(const std::string& symbol) override;
    void subscribe_orderbook() override;
    void subscribe_ticker() override;

    std::string orderbookSetup();

    
};