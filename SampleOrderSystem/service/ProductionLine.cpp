// service/ProductionLine.cpp
#include "service/ProductionLine.h"
#include "domain/OrderStatus.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>

using json = nlohmann::json;
namespace fs = std::filesystem;

ProductionLine::ProductionLine(InventoryService& inventory, IOrderRepository& order_repo,
                                IClockProvider& clock, const std::string& file_path)
    : inventory_(inventory), order_repo_(order_repo), clock_(clock), file_path_(file_path) {
    load();
}

void ProductionLine::load() {
    if (file_path_.empty()) return;
    std::ifstream f(file_path_);
    if (!f.is_open()) return;
    json j; f >> j;

    auto makeJob = [&](const json& jj) {
        ProductionJob job(jj["order_id"].get<int>(),
                          jj["sample_id"].get<int>(),
                          jj["actual_qty"].get<int>(),
                          jj["avg_production_time"].get<double>(),
                          clock_,
                          jj.value("shortfall", 0));
        job.addProduced(jj["produced_qty"].get<int>());
        job.resetLastTick();
        return job;
    };

    if (!j["current_job"].is_null()) {
        current_job_ = std::make_unique<ProductionJob>(makeJob(j["current_job"]));
    }
    for (auto& jj : j["waiting_queue"]) {
        waiting_queue_.push(makeJob(jj));
    }
}

void ProductionLine::persist() const {
    if (file_path_.empty()) return;
    auto parent = fs::path(file_path_).parent_path();
    if (!parent.empty()) fs::create_directories(parent);

    auto jobToJson = [](const ProductionJob& job) {
        return json{{"order_id",            job.getOrderId()},
                    {"sample_id",           job.getSampleId()},
                    {"actual_qty",          job.getActualQty()},
                    {"produced_qty",        job.getProducedQty()},
                    {"shortfall",           job.getShortfall()},
                    {"avg_production_time", job.getAvgProductionTime()}};
    };

    json j;
    j["current_job"] = current_job_ ? jobToJson(*current_job_) : json(nullptr);
    j["waiting_queue"] = json::array();
    auto q = waiting_queue_;
    while (!q.empty()) { j["waiting_queue"].push_back(jobToJson(q.front())); q.pop(); }

    std::ofstream(file_path_) << j.dump(2);
}

void ProductionLine::enqueue(int order_id, int sample_id,
                              int actual_qty, double avg_production_time, int shortfall) {
    ProductionJob job(order_id, sample_id, actual_qty, avg_production_time, clock_, shortfall);
    if (!current_job_) current_job_ = std::make_unique<ProductionJob>(job);
    else waiting_queue_.push(job);
    persist();
}

void ProductionLine::tick() {
    if (!current_job_) return;

    int newly = current_job_->calcNewlyProduced();

    if (newly > 0) {
        current_job_->addProduced(newly);
        inventory_.addActualStock(current_job_->getSampleId(), newly);
        persist();
    }

    if (current_job_->isComplete()) {
        Order* order = order_repo_.findById(current_job_->getOrderId());
        if (order) {
            order->setStatus(OrderStatus::CONFIRMED);
            order_repo_.save(*order);
        }
        current_job_.reset();
        startNextJob();
        persist();
    }
}

bool ProductionLine::isProducing() const { return current_job_ != nullptr; }
const ProductionJob* ProductionLine::getCurrentJob() const { return current_job_.get(); }
int ProductionLine::getQueueSize() const { return static_cast<int>(waiting_queue_.size()); }
std::queue<ProductionJob> ProductionLine::getWaitingQueue() const { return waiting_queue_; }

void ProductionLine::startNextJob() {
    if (!waiting_queue_.empty()) {
        current_job_ = std::make_unique<ProductionJob>(waiting_queue_.front());
        current_job_->resetLastTick();
        waiting_queue_.pop();
    }
}
