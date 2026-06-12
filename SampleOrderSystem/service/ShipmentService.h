// service/ShipmentService.h
#pragma once
#include "interface/IOrderRepository.h"
#include "service/InventoryService.h"

class ShipmentService {
public:
    ShipmentService(IOrderRepository& order_repo, InventoryService& inventory);

    bool release(int order_id);
    std::vector<Order*> getConfirmedOrders();

private:
    IOrderRepository& order_repo_;
    InventoryService& inventory_;
};
