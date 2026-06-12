// domain/Order.h
#pragma once
#include <string>
#include "domain/OrderStatus.h"

class Order {
public:
    Order(int id, int sample_id, const std::string& customer_name, int quantity);

    int getId() const;
    int getSampleId() const;
    const std::string& getCustomerName() const;
    int getQuantity() const;
    OrderStatus getStatus() const;
    void setStatus(OrderStatus status);

private:
    int id_;
    int sample_id_;
    std::string customer_name_;
    int quantity_;
    OrderStatus status_;
};
