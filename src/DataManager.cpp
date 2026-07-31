#include "DataManager.hpp"
#include <chrono>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/write.hpp>

MarketDataManager::MarketDataManager() {}

void MarketDataManager::enableLiveBroadcast(const std::string& host, int port) {
    liveBroadcastHost_ = host;
    liveBroadcastPort_ = port;
    liveBroadcastEnabled_ = true;
}

void MarketDataManager::setAnalyticsExportPath(const std::string& path) {
    analyticsExportPath_ = path;
}

namespace {
std::string serializeSnapshot(const std::string& product, const InstrumentSnapshot& snapshot) {
    nlohmann::json payload = nlohmann::json::object();
    payload["type"] = "analytics_update";
    payload["product"] = product;
    payload["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(snapshot.lastTickTime_.time_since_epoch()).count();
    payload["price"] = snapshot.lastPrice_;
    payload["buyVolume"] = snapshot.buyVolume_;
    payload["sellVolume"] = snapshot.sellVolume_;
    payload["tradesLastMinute"] = snapshot.tradesLastMinute_;
    payload["mid"] = snapshot.mid_;
    payload["spread"] = snapshot.spread_;
    payload["variance"] = snapshot.variance_;
    payload["vol30s"] = snapshot.vol30s_;
    payload["vol5m"] = snapshot.vol5m_;
    return payload.dump();
}
}

void MarketDataManager::exportAnalyticsSnapshot(const std::string& product) {
    auto analyticsIt = analytics_.find(product);
    if (analyticsIt == analytics_.end()) {
        return;
    }

    auto snapshotOpt = analyticsIt->second.getSnap();
    if (!snapshotOpt.has_value()) {
        return;
    }

    const InstrumentSnapshot& snapshot = snapshotOpt.value();
    const std::string payload = serializeSnapshot(product, snapshot);

    if (liveBroadcastEnabled_) {
        try {
            boost::asio::io_context io;
            boost::asio::ip::tcp::socket socket(io);
            boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::make_address(liveBroadcastHost_), liveBroadcastPort_);
            socket.connect(endpoint);
            boost::asio::write(socket, boost::asio::buffer(payload + "\n"));
        } catch (const std::exception&) {
            // Ignore broadcast failures so the app still runs when no listener is attached.
        }
    }

    {
        std::lock_guard<std::mutex> lock(analyticsHistoryMutex_);
        analyticsHistory_[product].push_back(snapshot);

        nlohmann::json history = nlohmann::json::array();
        for (const auto& entry : analyticsHistory_[product]) {
            history.push_back(nlohmann::json{{
                "product", product},
                {"timestamp", std::chrono::duration_cast<std::chrono::milliseconds>(entry.lastTickTime_.time_since_epoch()).count()},
                {"price", entry.lastPrice_},
                {"buyVolume", entry.buyVolume_},
                {"sellVolume", entry.sellVolume_},
                {"tradesLastMinute", entry.tradesLastMinute_},
                {"mid", entry.mid_},
                {"spread", entry.spread_},
                {"variance", entry.variance_},
                {"vol30s", entry.vol30s_},
                {"vol5m", entry.vol5m_}
            });
        }

        std::filesystem::path exportPath(analyticsExportPath_);
        if (!exportPath.parent_path().empty()) {
            std::error_code ec;
            std::filesystem::create_directories(exportPath.parent_path(), ec);
        }

        std::ofstream out(exportPath);
        if (!out.is_open()) {
            return;
        }

        out << history.dump(2);
    }
}

void MarketDataManager::addSpotTick(const std::string& product, SpotTick tick) {
    {
        std::lock_guard<std::mutex> lock(spotMutex_);
        auto& ticks = spotTicks_[product];
        ticks.push(tick);
        while (ticks.size() > maxSpotTicks_) {
            ticks.pop();
        }
    }

    auto analyticsIt = analytics_.find(product);
    if (analyticsIt != analytics_.end()) {
        analyticsIt->second.update(tick);
    } else {
        analytics_.emplace(product, AnalyticsEngine(tick));
        analytics_.find(product)->second.update(tick);
    }

    exportAnalyticsSnapshot(product);
}

SpotTick MarketDataManager::getLatestSpotTick(const std::string& product) {
    std::lock_guard<std::mutex> lock(spotMutex_);

    auto it = spotTicks_.find(product);
    if (it == spotTicks_.end() || it->second.empty()) {
        return SpotTick { 0, 0, "None", std::chrono::system_clock::now()};
    }

    return it->second.top();
}

void MarketDataManager::addOptionTick(const OptionTick& tick, const std::string& key) {
    optionTicks_[key] = tick;
}

void MarketDataManager::updateOrderBook(const OrderBookEvent& deltas, const std::string& key) {
    {
        std::lock_guard<std::mutex> lock(obMutex_);
        for (const auto& [price, quantity] : deltas.newAsks) {
            ob_.asks_.insert_or_assign(price, quantity);
        }

        for (const auto& [price, quantity] : deltas.newBids) {
            ob_.bids_.insert_or_assign(price, quantity);
        }
    }

    auto analyticsIt = analytics_.find(key);
    if (analyticsIt == analytics_.end()) {
        SpotTick seededTick{0.0, 0.0, "none", std::chrono::system_clock::now()};
        analytics_.emplace(key, AnalyticsEngine(seededTick));
        analyticsIt = analytics_.find(key);
    }

    analyticsIt->second.update(ob_);
}
 
OptionTick MarketDataManager::getOptionTick(const std::string& key) {
    std::lock_guard<std::mutex> lock(optionMutex_);
    auto it = optionTicks_.find(key);
    if (it != optionTicks_.end()) {
        return it->second;
    }
    return OptionTick{0, 0, 0, std::chrono::system_clock::now()};
}

void MarketDataManager::processMarketEvent(MarketEvent payload) {
    if (std::holds_alternative<TickEvent>(payload)) {
        TickEvent tick_event = std::get<TickEvent>(payload);
        SpotTick tick;
        tick.timestamp = tick_event.timestamp;
        tick.price = tick_event.price;
        tick.size = tick_event.size;
        tick.side = tick_event.side;
        tick.buyAmt = tick_event.buyAmt;
        tick.sellAmt = tick_event.sellAmt;
        tick.tradedAmt = tick_event.tradedAmt;
        tick.bestAsk = tick_event.bestAsk;
        tick.bestBid = tick_event.bestBid;
        addSpotTick(tick_event.instrument, tick);
    } else if (std::holds_alternative<OrderBookEvent>(payload)) {
        OrderBookEvent ob_event = std::get<OrderBookEvent>(payload);
        updateOrderBook(ob_event, ob_event.instrument);
    }
}

void MarketDataManager::processMessage(MarketEvent payload) {
    processMarketEvent(std::move(payload));
}

void MarketDataManager::tick() {
}
