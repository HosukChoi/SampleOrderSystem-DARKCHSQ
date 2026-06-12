// controller/AppController.cpp
#include "controller/AppController.h"

AppController::AppController()
    : sample_repo_("data/samples.json"),
      order_repo_("data/orders.json"),
      inventory_(order_repo_, "data/inventory.json"),
      sample_svc_(sample_repo_),
      production_line_(inventory_, order_repo_, clock_, "data/production_queue.json"),
      order_svc_(order_repo_, sample_repo_, inventory_, production_line_),
      shipment_svc_(order_repo_, inventory_),
      main_view_(sample_svc_, order_svc_, inventory_, production_line_, shipment_svc_) {}

void AppController::run() {
    while (true) {
        production_line_.tick();
        int choice = main_view_.promptMainMenu();
        switch (choice) {
            case 1: handleSample();        break;
            case 2: handlePlaceOrder();    break;
            case 3: handleApprovalOrder(); break;
            case 4: handleMonitoring();    break;
            case 5: handleProduction();    break;
            case 6: handleShipment();      break;
            case 0: return;
        }
        production_line_.tick();
    }
}

void AppController::handleSample() {
    SampleView v(sample_svc_, inventory_);
    v.run();
}

void AppController::handlePlaceOrder() {
    OrderView v(order_svc_, sample_svc_);
    v.placeOrder();
}

void AppController::handleApprovalOrder() {
    OrderView v(order_svc_, sample_svc_);
    v.processApproval();
}

void AppController::handleMonitoring() {
    MonitoringView v(order_svc_, sample_svc_, inventory_, production_line_);
    v.render();
}

void AppController::handleShipment() {
    ShipmentView v(shipment_svc_, sample_svc_);
    v.run();
}

void AppController::handleProduction() {
    ProductionView v(production_line_, sample_svc_, order_svc_);
    v.run();
}
