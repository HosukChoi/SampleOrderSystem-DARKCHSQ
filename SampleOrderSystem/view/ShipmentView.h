// view/ShipmentView.h
#pragma once
#include "service/ShipmentService.h"

class ShipmentView {
public:
    explicit ShipmentView(ShipmentService& shipment_svc);
    void run();

private:
    ShipmentService& shipment_svc_;
};
