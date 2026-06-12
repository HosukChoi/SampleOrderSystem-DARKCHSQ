// interface/IOrderRepository.h
#pragma once
#include <vector>
#include "interface/IComponent.h"
#include "domain/Order.h"
#include "domain/OrderStatus.h"

interface IOrderRepository : public IComponent {
    virtual void save(const Order& order) = 0;
    virtual Order* findById(int id) = 0;
    virtual std::vector<Order*> findAll() = 0;
    virtual std::vector<Order*> findByStatus(OrderStatus status) = 0;
    virtual int nextId() = 0;
    virtual int getId() const override = 0;
};
