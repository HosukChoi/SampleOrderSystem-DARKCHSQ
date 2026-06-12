// domain/ProductionJob.cpp
#include "domain/ProductionJob.h"
#include <algorithm>
#include <cmath>

ProductionJob::ProductionJob(int order_id, int sample_id, int actual_qty,
                              double avg_production_time, IClockProvider& clock,
                              int shortfall)
    : order_id_(order_id), sample_id_(sample_id), actual_qty_(actual_qty),
      produced_qty_(0), shortfall_(shortfall), avg_production_time_(avg_production_time),
      clock_(clock), last_tick_(clock.now()), accumulated_time_(0.0) {}

int ProductionJob::getOrderId() const { return order_id_; }
int ProductionJob::getSampleId() const { return sample_id_; }
int ProductionJob::getActualQty() const { return actual_qty_; }
int ProductionJob::getProducedQty() const { return produced_qty_; }
int ProductionJob::getShortfall() const { return shortfall_; }
double ProductionJob::getAvgProductionTime() const { return avg_production_time_; }
void ProductionJob::addProduced(int qty) { produced_qty_ += qty; }
bool ProductionJob::isComplete() const { return produced_qty_ >= actual_qty_; }

int ProductionJob::calcNewlyProduced() {
    auto now = clock_.now();
    double elapsed = std::chrono::duration<double>(now - last_tick_).count();
    last_tick_ = now;
    accumulated_time_ += elapsed;
    int newly = static_cast<int>(accumulated_time_ / avg_production_time_);
    accumulated_time_ -= newly * avg_production_time_;
    return std::min(newly, actual_qty_ - produced_qty_);
}

void ProductionJob::resetLastTick() {
    last_tick_ = clock_.now();
    accumulated_time_ = 0.0;
}
