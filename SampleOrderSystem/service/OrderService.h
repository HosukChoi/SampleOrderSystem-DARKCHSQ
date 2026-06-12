// service/OrderService.h
#pragma once
#include "interface/IOrderRepository.h"
#include "interface/ISampleRepository.h"
#include "service/InventoryService.h"
#include "service/ProductionLine.h"

class OrderService {
public:
    OrderService(IOrderRepository& order_repo, ISampleRepository& sample_repo,
                 InventoryService& inventory, ProductionLine& production_line);

    Order* placeOrder(int sample_id, const std::string& customer_name, int quantity);
    bool approveOrder(int order_id);
    bool rejectOrder(int order_id);
    std::vector<Order*> getReservedOrders();
    std::vector<Order*> getAllOrders();

private:
    IOrderRepository&  order_repo_;
    ISampleRepository& sample_repo_;
    InventoryService&  inventory_;
    ProductionLine&    production_line_;
};
