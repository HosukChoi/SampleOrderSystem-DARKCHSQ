// service/ProductionLine.h
#pragma once
#include <queue>
#include <memory>
#include "domain/ProductionJob.h"
#include "service/InventoryService.h"
#include "interface/IOrderRepository.h"
#include "interface/IClockProvider.h"

class ProductionLine {
public:
    ProductionLine(InventoryService& inventory, IOrderRepository& order_repo,
                   IClockProvider& clock);

    void enqueue(int order_id, int sample_id, int actual_qty, double avg_production_time);
    void tick();
    bool isProducing() const;
    const ProductionJob* getCurrentJob() const;
    int getQueueSize() const;
    std::queue<ProductionJob> getWaitingQueue() const;

private:
    InventoryService&              inventory_;
    IOrderRepository&              order_repo_;
    IClockProvider&                clock_;
    std::unique_ptr<ProductionJob> current_job_;
    std::queue<ProductionJob>      waiting_queue_;

    void startNextJob();
};
