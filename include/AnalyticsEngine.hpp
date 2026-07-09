#pragma once
#include <string>
#include "DataContainers.hpp"

class RollingVWAP {
    
};

class RollingVol {

};

class SpreadMetrics {
    double getMicroprice();
    double getSpread();
    double getMid();
};

class OrderbookMetrics {
    double getImbalance();
};

struct TradeMetrics {
    double lastTick_;
    double buyVolumne_;
    double sellVolume_;
    int tradesLastMinute_;
};

class InstrumentSnapshot {
    private:
        double lastTick_;
        double buyVolume_;
        double sellVolume_;
        int tradesLastMinute_;

        double orderbookImbalance_;


        double vol30s_;
        double vol5m_;
        double vwap_;

    public:
        OrderbookMetrics orderbook_;
        SpreadMetrics spread_; 
};

class AnalyticsEngine {
    private:
        double getReturns(const std::string& product);
        InstrumentSnapshot snapshot_;

    public:

        getSnap(const SpotTick& tick);
        RollingVol vol30s_;
        RollingVol vol5m_;
        RollingVWAP vwap_;

};