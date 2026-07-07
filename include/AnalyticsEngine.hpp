#pragma once

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

class AnalyticsEngine {
    private:
        double getReturns(const std::string& product);



};