// view/ProductionView.h
#pragma once
#include "service/ProductionLine.h"
#include "service/SampleService.h"
#include "service/OrderService.h"

class ProductionView {
public:
    ProductionView(ProductionLine& production_line,
                   SampleService& sample_svc, OrderService& order_svc);
    void run();

private:
    ProductionLine& production_line_;
    SampleService&  sample_svc_;
    OrderService&   order_svc_;
};
