#include "AnalyticsEngine.hpp"
#include <cmath>
#include <optional>

double RollingVar::getVar() const {
    return variance30s_;
}


void RollingVar::onReturn(double ret, bool snapshotReady) {
    auto now = std::chrono::system_clock::now();

    std::chrono::duration<double> t30s = now - interval30s_;
    std::chrono::duration<double> t5m = now - interval5m_;
    double ret_sq = ret * ret;

    if (t30s.count() > 30.0) { 
        variance30s_ = decay_ * variance30s_ + (1 - decay_) * ret_sq;
    } else if (snapshotReady) {
        variance30s_ = ret_sq;
    }

    if (t5m.count() > 300.0) {
        variance5m_ = decay_ * variance5m_ + (1 - decay_) * ret_sq;
    } else if (snapshotReady) {
        variance5m_ = ret_sq;
    }
}

double RollingVar::vol30s() const {
    return std::sqrt(variance30s_);
}

double RollingVar::vol5m() const {
    return std::sqrt(variance5m_);
}


AnalyticsEngine::AnalyticsEngine(const SpotTick& tick) : snapshotReady_{false}, lastTick_{tick}, varMetrics_{}, vwap_{}, snapshot_{}, ob_{} {

}

double AnalyticsEngine::getReturns(const SpotTick& tick){
    double ret = tick.price / lastTick_.price;
    double logRet = std::log(ret);

    return logRet;
}

std::optional<InstrumentSnapshot> AnalyticsEngine::getSnap() {
    if (!snapshotReady_) { return std::nullopt;}

    return snapshot_;

}

void AnalyticsEngine::update(const SpotTick& tick) {

    double log_ret = getReturns(tick);
    varMetrics_.onReturn(log_ret, snapshotReady_); 

    snapshot_.lastTickTime_ = tick.timestamp;
    snapshot_.buyVolume_ = tick.buyAmt;
    snapshot_.sellVolume_ = tick.sellAmt;
    snapshot_.tradesLastMinute_ = tick.tradedAmt;
    snapshot_.lastPrice_ = tick.price;
    snapshot_.mid_ = (tick.bestAsk + tick.bestBid) / 2.0;
    snapshot_.spread_ = (tick.bestAsk - tick.bestBid);
    snapshot_.variance_ = varMetrics_.getVar();
    
    snapshot_.vol30s_ = varMetrics_.vol30s();
    snapshot_.vol5m_ = varMetrics_.vol5m();

    snapshotReady_ = true;
}

void AnalyticsEngine::update(const OrderBookTick& tick) {

    for (auto ask : tick.newAsks_) {
        if (ask.second == 0.0) {
            ob_.asks_.erase(ask.first);
        } else {
            ob_.asks_.insert(ask);
        } 
    }

    for (auto bid : tick.newBids_) {
        if (bid.second == 0.0) {
            ob_.bids_.erase(bid.first);
        } else {
            ob_.bids_.insert(bid);  
        }  
    }    

    if (!tick.newAsks_.empty() || !tick.newBids_.empty()) {

        auto best_bid = ob_.bids_.begin();
        auto best_ask = ob_.asks_.begin();
        
        ob_.spread_ = best_ask->first - best_bid->first;
        ob_.depth_ = ob_.asks_.crbegin()->first - ob_.bids_.crbegin()->first;
        ob_.imbalance_ = best_bid->second / (best_ask->second + best_bid->second);

        // Simple microprice for now
        ob_.microprice_ = ((best_ask->first * best_ask->second) + (best_bid->first * best_bid->second)) / (best_bid->second + best_ask->second);

    }

}
