// domain/ProductionJob.h
#pragma once
#include <chrono>
#include "interface/IClockProvider.h"

class ProductionJob {
public:
    ProductionJob(int order_id, int sample_id, int actual_qty,
                  double avg_production_time, IClockProvider& clock,
                  int shortfall = 0);

    int getOrderId() const;
    int getSampleId() const;
    int getActualQty() const;
    int getProducedQty() const;
    int getShortfall() const;
    double getAvgProductionTime() const;
    void addProduced(int qty);
    bool isComplete() const;
    int calcNewlyProduced();
    void resetLastTick();

private:
    int order_id_;
    int sample_id_;
    int actual_qty_;
    int produced_qty_;
    int shortfall_;
    double avg_production_time_;
    IClockProvider& clock_;
    std::chrono::system_clock::time_point last_tick_;
    double accumulated_time_;
};
