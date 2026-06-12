// service/OrderService.cpp
#include "service/OrderService.h"
#include <cmath>

OrderService::OrderService(IOrderRepository& order_repo, ISampleRepository& sample_repo,
                           InventoryService& inventory, ProductionLine& production_line)
    : order_repo_(order_repo), sample_repo_(sample_repo),
      inventory_(inventory), production_line_(production_line) {}

Order* OrderService::placeOrder(int sample_id, const std::string& customer_name, int quantity) {
    if (sample_repo_.findById(sample_id) == nullptr) return nullptr;
    int id = order_repo_.nextId();
    order_repo_.save(Order(id, sample_id, customer_name, quantity));
    return order_repo_.findById(id);
}

bool OrderService::approveOrder(int order_id) {
    Order* order = order_repo_.findById(order_id);
    if (!order || order->getStatus() != OrderStatus::RESERVED) return false;

    int available = inventory_.getAvailableStock(order->getSampleId());

    if (available >= order->getQuantity()) {
        order->setStatus(OrderStatus::CONFIRMED);
    } else {
        order->setStatus(OrderStatus::PRODUCING);
        Sample* sample = sample_repo_.findById(order->getSampleId());
        int shortfall  = order->getQuantity() - available;
        int actual_qty = static_cast<int>(
            std::ceil(shortfall / (sample->getYield() * 0.9)));
        production_line_.enqueue(order_id, order->getSampleId(),
                                 actual_qty, sample->getAvgProductionTime());
    }
    order_repo_.save(*order);
    return true;
}

bool OrderService::rejectOrder(int order_id) {
    Order* order = order_repo_.findById(order_id);
    if (!order || order->getStatus() != OrderStatus::RESERVED) return false;
    order->setStatus(OrderStatus::REJECTED);
    order_repo_.save(*order);
    return true;
}

std::vector<Order*> OrderService::getReservedOrders() {
    return order_repo_.findByStatus(OrderStatus::RESERVED);
}

std::vector<Order*> OrderService::getAllOrders() {
    return order_repo_.findAll();
}
