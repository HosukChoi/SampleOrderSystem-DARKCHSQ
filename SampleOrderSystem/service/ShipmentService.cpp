// service/ShipmentService.cpp
#include "service/ShipmentService.h"

ShipmentService::ShipmentService(IOrderRepository& order_repo, InventoryService& inventory)
    : order_repo_(order_repo), inventory_(inventory) {}

bool ShipmentService::release(int order_id) {
    Order* order = order_repo_.findById(order_id);
    if (!order || order->getStatus() != OrderStatus::CONFIRMED) return false;
    inventory_.subtractActualStock(order->getSampleId(), order->getQuantity());
    order->setStatus(OrderStatus::RELEASE);
    order_repo_.save(*order);
    return true;
}

std::vector<Order*> ShipmentService::getConfirmedOrders() {
    return order_repo_.findByStatus(OrderStatus::CONFIRMED);
}
