// view/MonitoringView.h
#pragma once
#include "service/OrderService.h"
#include "service/SampleService.h"
#include "service/InventoryService.h"
#include "service/ProductionLine.h"

class MonitoringView {
public:
    MonitoringView(OrderService& order_svc, SampleService& sample_svc,
                   InventoryService& inventory, ProductionLine& production_line);
    void render();

private:
    OrderService& order_svc_;
    SampleService& sample_svc_;
    InventoryService& inventory_;
    ProductionLine& production_line_;
};
