// view/MainView.h
#pragma once
#include "service/SampleService.h"
#include "service/OrderService.h"
#include "service/InventoryService.h"
#include "service/ProductionLine.h"
#include "service/ShipmentService.h"

class MainView {
public:
    MainView(SampleService& sample_svc, OrderService& order_svc,
             InventoryService& inventory, ProductionLine& production_line,
             ShipmentService& shipment_svc);
    int promptMainMenu();
    void printSummary();

private:
    SampleService& sample_svc_;
    OrderService& order_svc_;
    InventoryService& inventory_;
    ProductionLine& production_line_;
    ShipmentService& shipment_svc_;
};
