// service/InventoryService.cpp
#include "service/InventoryService.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>

using json = nlohmann::json;
namespace fs = std::filesystem;

InventoryService::InventoryService(IOrderRepository& order_repo,
                                   const std::string& filePath)
    : order_repo_(order_repo), file_path_(filePath) {
    load();
}

void InventoryService::load() {
    std::ifstream f(file_path_);
    if (!f.is_open()) return;
    json j; f >> j;
    for (auto& [key, val] : j.items())
        actual_stock_[std::stoi(key)] = val.get<int>();
}

void InventoryService::persist() const {
    auto parent = fs::path(file_path_).parent_path();
    if (!parent.empty()) fs::create_directories(parent);
    json j;
    for (auto& [id, qty] : actual_stock_)
        j[std::to_string(id)] = qty;
    std::ofstream(file_path_) << j.dump(2);
}

int InventoryService::getActualStock(int sample_id) const {
    auto it = actual_stock_.find(sample_id);
    return it != actual_stock_.end() ? it->second : 0;
}

void InventoryService::addActualStock(int sample_id, int qty) {
    actual_stock_[sample_id] += qty;
    persist();
}

void InventoryService::subtractActualStock(int sample_id, int qty) {
    actual_stock_[sample_id] -= qty;
    persist();
}

int InventoryService::getRequiredStock(int sample_id) const {
    int total = 0;
    for (auto* order : order_repo_.findAll()) {
        if (order->getSampleId() == sample_id &&
            (order->getStatus() == OrderStatus::CONFIRMED ||
             order->getStatus() == OrderStatus::PRODUCING)) {
            total += order->getQuantity();
        }
    }
    return total;
}

int InventoryService::getAvailableStock(int sample_id) const {
    return getActualStock(sample_id) - getRequiredStock(sample_id);
}

const char* InventoryService::getStockStatus(int sample_id) const {
    if (getActualStock(sample_id) == 0)    return "고갈";
    if (getAvailableStock(sample_id) <= 0) return "부족";
    return "여유";
}
