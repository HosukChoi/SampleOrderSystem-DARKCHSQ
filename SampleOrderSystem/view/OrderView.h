// view/OrderView.h
#pragma once
#include "service/OrderService.h"
#include "service/SampleService.h"

class OrderView {
public:
    OrderView(OrderService& order_svc, SampleService& sample_svc);
    void run();

private:
    OrderService& order_svc_;
    SampleService& sample_svc_;
    void placeOrder();
    void processApproval();
};
