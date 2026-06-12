// controller/AppController.h
#pragma once
#include "repository/JsonSampleRepository.h"
#include "repository/JsonOrderRepository.h"
#include "service/RealClock.h"
#include "service/InventoryService.h"
#include "service/SampleService.h"
#include "service/OrderService.h"
#include "service/ProductionLine.h"
#include "service/ShipmentService.h"
#include "view/MainView.h"
#include "view/SampleView.h"
#include "view/OrderView.h"
#include "view/MonitoringView.h"
#include "view/ProductionView.h"
#include "view/ShipmentView.h"

class AppController {
public:
    AppController();
    void run();

private:
    JsonSampleRepository sample_repo_;
    JsonOrderRepository  order_repo_;
    RealClock            clock_;
    InventoryService     inventory_;
    SampleService        sample_svc_;
    ProductionLine       production_line_;
    OrderService         order_svc_;
    ShipmentService      shipment_svc_;
    MainView             main_view_;

    void handleSample();
    void handleOrder();
    void handleMonitoring();
    void handleShipment();
    void handleProduction();
};
