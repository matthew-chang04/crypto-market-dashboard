#include "WebSocketClient.hpp"
#include <nlohmann/json.hpp>
#include <string>
#include <set>
#include <chrono>
#include <ctime>
#include <iomanip>

using namespace std::chrono;

// TODO: IMPLEMENT the functionality with deribit get_instruments to pull all options for a coin, and then filter through those using the "strike" and "expiration timestamp" feilds
class DeribitClient : public WebSocketClient
{
public:
    const static std::string HOST;
    const static std::string PORT;

    DeribitClient(net::io_context& ioc, net::ssl::context& sslCtx, tcp::resolver& resolver, std::string& target, std::string& symbol);

    const std::string& normalizeSymbol(const std::string& symbol) override;
    std::string format_date(std::chrono::system_clock::time_point day);
    std::string create_symbol(const std::string& base, const std::string& expiry, double strike);

    void subscribe_ticker(const std::string& symbol) override;
    void unsubscribe_ticker(const std::string& symbol) override;

    void subscribe_orderbook(const std::string& symbol) override;
    void subscribe_tracked(double spotPrice);
    std::optional<MarketEvent> parsePayload(const std::string& msg) override;
    std::string buildRequestMsg(const std::string& action, const std::string& product) override;

    std::vector<std::string> trackedExpiries_;
    std::set<std::string> subscribedTickers_;

private:
    int nextId_ = 1;
};
