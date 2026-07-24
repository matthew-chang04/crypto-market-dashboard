#pragma once
#include <string>
#include <chrono>
#include "DataContainers.hpp"

class RollingVWAP {

};

class RollingVar {
    public:
        void onReturn(double ret, bool snapshotReady);
        double vol30s() const;
        double vol5m() const;
        
        double getVar() const;
    private:
        std::chrono::system_clock::time_point interval30s_ = std::chrono::system_clock::now();
        std::chrono::system_clock::time_point interval5m_ = std::chrono::system_clock::now(); 
        double decay_ = 0.94;
        double parkinson_;
        double variance30s_;
        double variance5m_;
};


class OrderbookMetrics {
    double getMid();
    double getSpread();
    double getMicroprice();
    double getImbalance();
};

struct TradeMetrics {
    double lastTick_;
    double buyVolumne_;
    double sellVolume_;
    int tradesLastMinute_;
};

struct InstrumentSnapshot {
    
    std::chrono::system_clock::time_point lastTickTime_;
    double buyVolume_;
    double sellVolume_;
    int tradesLastMinute_;

    double microPrice_;
    double spread_;
    double mid_;

    double orderbookImbalance_;

    double variance_;
    double vol30s_;
    double vol5m_;
    double vwap_;
    double lastPrice_;

};

class AnalyticsEngine {
    private:
        bool snapshotReady_;
        SpotTick lastTick_;

        RollingVar varMetrics_;
        RollingVWAP vwap_;
        double getReturns(const SpotTick& tick);
        InstrumentSnapshot snapshot_;

    public:
        AnalyticsEngine(const SpotTick& tick);
        std::optional<InstrumentSnapshot> getSnap();
        void update(const SpotTick& tick);

};