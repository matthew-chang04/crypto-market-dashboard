#include "AnalyticsEngine.hpp"
#include <cmath>
#include <optional>


void RollingVar::onReturn(double ret) {
    auto now = std::chrono::system_clock::now();
    // if (now - interval30s_ > 30000.0)
    double ret_sq = ret * ret;
    variance30s_ = decay_ * variance30s_ + (1 - decay_) * ret_sq;
    variance5m_ = decay_ * variance5m_ + (1 - decay_) * ret_sq;
}

double RollingVar::vol30s() const {
    return std::sqrt(variance30s_);
}

double RollingVar::vol5m() const {
    return std::sqrt(variance5m_)
}


AnalyticsEngine::AnalyticsEngine(const SpotTick& tick) : snapshotReady_{false}, lastTick_{tick}, varMetrics_{}, vwap_{}, snapshot_{} {

}

double AnalyticsEngine::getReturns(const SpotTick& tick){
    double ret = tick.price / lastTick_.price;
    double logRet = std::log(ret);

    return logRet;
}

std::optional<InstrumentSnapshot> AnalyticsEngine::getSnap(const SpotTick& tick) {
    /* 
        get computations for metrics:
            - log returns
            - rolling vols
            - variance (for further covariance if needed)

    */

    if (!snapshotReady_) { return std::nullopt;}


    double logRet = getReturns(tick);
    varMetrics_.onReturn(logRet);


    InstrumentSnapshot ss{};

    ss.lastTickTime_ = tick.timestamp;
    ss.buyVolume_ = tick.buyAmt;
    ss.sellVolume_ = tick.sellAmt;
    ss.tradesLastMinute_ = tick.tradedAmt;
    ss.spread_ = (tick.bestAsk - tick.bestBid);
    ss.variance_ = varMetrics_.getVar();

    ss.vol30s_ = varMetrics_.vol30s();
    ss.vol5m_ = varMetrics_.vol5m();
    ss.vwap_ = 

}

