// service/ProductionLine.cpp — stub (Task 8에서 완성)
#include "service/ProductionLine.h"

ProductionLine::ProductionLine(InventoryService& inventory, IOrderRepository& order_repo,
                                IClockProvider& clock)
    : inventory_(inventory), order_repo_(order_repo), clock_(clock) {}

void ProductionLine::enqueue(int order_id, int sample_id,
                              int actual_qty, double avg_production_time) {
    ProductionJob job(order_id, sample_id, actual_qty, avg_production_time, clock_);
    if (!current_job_) current_job_ = std::make_unique<ProductionJob>(job);
    else waiting_queue_.push(job);
}

void ProductionLine::tick() {}  // Task 8에서 구현
bool ProductionLine::isProducing() const { return current_job_ != nullptr; }
const ProductionJob* ProductionLine::getCurrentJob() const { return current_job_.get(); }
int ProductionLine::getQueueSize() const { return static_cast<int>(waiting_queue_.size()); }
std::queue<ProductionJob> ProductionLine::getWaitingQueue() const { return waiting_queue_; }

void ProductionLine::startNextJob() {
    if (!waiting_queue_.empty()) {
        current_job_ = std::make_unique<ProductionJob>(waiting_queue_.front());
        waiting_queue_.pop();
    }
}
