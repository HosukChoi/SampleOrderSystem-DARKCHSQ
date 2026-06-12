// service/InventoryService.h
#pragma once
#include <map>
#include <string>
#include "interface/IOrderRepository.h"

class InventoryService {
public:
    explicit InventoryService(IOrderRepository& order_repo,
                              const std::string& filePath = "data/inventory.json");

    int getActualStock(int sample_id) const;
    void addActualStock(int sample_id, int qty);
    void subtractActualStock(int sample_id, int qty);
    int getRequiredStock(int sample_id) const;
    int getAvailableStock(int sample_id) const;
    const char* getStockStatus(int sample_id) const;

private:
    IOrderRepository&  order_repo_;
    std::string        file_path_;
    std::map<int, int> actual_stock_;

    void load();
    void persist() const;
};
