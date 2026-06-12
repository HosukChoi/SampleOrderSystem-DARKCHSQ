// repository/JsonOrderRepository.h
#pragma once
#include <map>
#include <string>
#include "interface/IOrderRepository.h"

class JsonOrderRepository : public IOrderRepository {
public:
    explicit JsonOrderRepository(const std::string& filePath);

    int getId() const override { return 2; }
    void save(const Order& order) override;
    Order* findById(int id) override;
    std::vector<Order*> findAll() override;
    std::vector<Order*> findByStatus(OrderStatus status) override;
    int nextId() override;

private:
    std::string      file_path_;
    std::map<int, Order> store_;
    int              next_id_ = 1;

    void load();
    void persist() const;
};
