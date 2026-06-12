// view/ShipmentView.h
#pragma once
#include "service/ShipmentService.h"
#include "service/SampleService.h"

class ShipmentView {
public:
    ShipmentView(ShipmentService& shipment_svc, SampleService& sample_svc);
    void run();

private:
    ShipmentService& shipment_svc_;
    SampleService&   sample_svc_;
};
