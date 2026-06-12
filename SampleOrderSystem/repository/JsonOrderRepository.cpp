// repository/JsonOrderRepository.cpp
#include "repository/JsonOrderRepository.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>

using json = nlohmann::json;
namespace fs = std::filesystem;

static OrderStatus strToStatus(const std::string& s) {
    if (s == "RESERVED")  return OrderStatus::RESERVED;
    if (s == "REJECTED")  return OrderStatus::REJECTED;
    if (s == "PRODUCING") return OrderStatus::PRODUCING;
    if (s == "CONFIRMED") return OrderStatus::CONFIRMED;
    return OrderStatus::RELEASE;
}

JsonOrderRepository::JsonOrderRepository(const std::string& filePath)
    : file_path_(filePath) {
    load();
}

void JsonOrderRepository::load() {
    std::ifstream f(file_path_);
    if (!f.is_open()) return;
    json j; f >> j;
    for (auto& item : j) {
        Order o(item["id"].get<int>(),
                item["sample_id"].get<int>(),
                item["customer_name"].get<std::string>(),
                item["quantity"].get<int>());
        o.setStatus(strToStatus(item["status"].get<std::string>()));
        int id = o.getId();
        store_.emplace(id, o);
        if (id >= next_id_) next_id_ = id + 1;
    }
}

void JsonOrderRepository::persist() const {
    fs::create_directories(fs::path(file_path_).parent_path());
    json j = json::array();
    for (auto& [id, o] : store_)
        j.push_back({{"id",            o.getId()},
                     {"sample_id",     o.getSampleId()},
                     {"customer_name", o.getCustomerName()},
                     {"quantity",      o.getQuantity()},
                     {"status",        toString(o.getStatus())}});
    std::ofstream(file_path_) << j.dump(2);
}

void JsonOrderRepository::save(const Order& order) {
    store_.emplace(order.getId(), order);
    persist();
}

Order* JsonOrderRepository::findById(int id) {
    auto it = store_.find(id);
    return it != store_.end() ? &it->second : nullptr;
}

std::vector<Order*> JsonOrderRepository::findAll() {
    std::vector<Order*> result;
    for (auto& [id, order] : store_)
        result.push_back(&order);
    return result;
}

std::vector<Order*> JsonOrderRepository::findByStatus(OrderStatus status) {
    std::vector<Order*> result;
    for (auto& [id, order] : store_)
        if (order.getStatus() == status)
            result.push_back(&order);
    return result;
}

int JsonOrderRepository::nextId() { return next_id_++; }
