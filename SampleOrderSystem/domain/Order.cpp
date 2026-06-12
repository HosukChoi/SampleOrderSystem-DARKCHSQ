// domain/Order.cpp
#include "domain/Order.h"

Order::Order(int id, int sample_id, const std::string& customer_name, int quantity)
    : id_(id), sample_id_(sample_id), customer_name_(customer_name),
      quantity_(quantity), status_(OrderStatus::RESERVED) {}

int Order::getId() const { return id_; }
int Order::getSampleId() const { return sample_id_; }
const std::string& Order::getCustomerName() const { return customer_name_; }
int Order::getQuantity() const { return quantity_; }
OrderStatus Order::getStatus() const { return status_; }
void Order::setStatus(OrderStatus status) { status_ = status; }
