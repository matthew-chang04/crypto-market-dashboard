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

    if (t5m.count() > 120.0) {
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


AnalyticsEngine::AnalyticsEngine(const SpotTick& tick) : snapshotReady_{false}, lastTick_{tick}, varMetrics_{}, vwap_{}, snapshot_{} {

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

