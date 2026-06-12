// view/OrderView.cpp
#include "view/OrderView.h"
#include "view/ConsoleUtils.h"
#include <iostream>
#include <cstdio>

OrderView::OrderView(OrderService& order_svc, SampleService& sample_svc)
    : order_svc_(order_svc), sample_svc_(sample_svc) {}

void OrderView::placeOrder() {
    ConsoleUtils::printHeader("주문 접수");
    int sample_id = ConsoleUtils::readInt("시료 ID: ");
    std::string customer = ConsoleUtils::readString("고객명: ");
    int qty = ConsoleUtils::readInt("주문 수량: ");
    Order* order = order_svc_.placeOrder(sample_id, customer, qty);
    if (order) printf("주문 접수 완료. 주문 ID: %d (RESERVED)\n", order->getId());
    else std::cout << "존재하지 않는 시료 ID입니다.\n";
    ConsoleUtils::pause();
}

void OrderView::processApproval() {
    ConsoleUtils::printHeader("주문 승인/거절");
    auto reserved = order_svc_.getReservedOrders();
    if (reserved.empty()) { std::cout << "접수된 주문이 없습니다.\n"; ConsoleUtils::pause(); return; }
    using P = ConsoleUtils;
    printf("%-4s %s %s %s\n",
           "ID",
           P::pad("고객명", 16).c_str(),
           P::pad("시료ID", 7).c_str(),
           "수량");
    ConsoleUtils::printSeparator();
    for (auto* o : reserved)
        printf("%-4d %s %-7d %dea\n",
               o->getId(),
               P::pad(o->getCustomerName(), 16).c_str(),
               o->getSampleId(), o->getQuantity());
    int order_id = ConsoleUtils::readInt("\n주문 ID: ");
    std::cout << "1. 승인  2. 거절  0. 취소\n";
    int action = ConsoleUtils::readInt("> ");
    if (action == 1)
        order_svc_.approveOrder(order_id) ? std::cout << "승인 처리되었습니다.\n" : std::cout << "처리 실패.\n";
    else if (action == 2)
        order_svc_.rejectOrder(order_id) ? std::cout << "거절 처리되었습니다.\n" : std::cout << "처리 실패.\n";
    ConsoleUtils::pause();
}
