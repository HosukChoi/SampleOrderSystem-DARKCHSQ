// view/MonitoringView.cpp
#include "view/MonitoringView.h"
#include "view/ConsoleUtils.h"
#include <iostream>
#include <cstdio>

MonitoringView::MonitoringView(OrderService& order_svc, SampleService& sample_svc,
                                InventoryService& inventory, ProductionLine& production_line)
    : order_svc_(order_svc), sample_svc_(sample_svc),
      inventory_(inventory), production_line_(production_line) {}

void MonitoringView::render() {
    while (true) {
        production_line_.tick();
        ConsoleUtils::clearScreen();
        ConsoleUtils::printHeader("모니터링");
        std::cout << "1. 주문량 확인\n2. 재고량 확인\n0. 뒤로\n";
        int choice = ConsoleUtils::readInt("> ");
        if (choice == 0) return;
        if (choice == 1) showOrderStatus();
        else if (choice == 2) showInventoryStatus();
    }
}

void MonitoringView::showOrderStatus() {
    production_line_.tick();
    ConsoleUtils::clearScreen();
    ConsoleUtils::printHeader("주문 현황");

    auto all = order_svc_.getAllOrders();
    int cnt[4] = {};
    for (auto* o : all) {
        if (o->getStatus() == OrderStatus::RESERVED)  cnt[0]++;
        if (o->getStatus() == OrderStatus::PRODUCING) cnt[1]++;
        if (o->getStatus() == OrderStatus::CONFIRMED) cnt[2]++;
        if (o->getStatus() == OrderStatus::RELEASE)   cnt[3]++;
    }
    printf("  RESERVED  : %d건\n", cnt[0]);
    printf("  PRODUCING : %d건\n", cnt[1]);
    printf("  CONFIRMED : %d건\n", cnt[2]);
    printf("  RELEASE   : %d건\n", cnt[3]);
    ConsoleUtils::pause();
}

void MonitoringView::showInventoryStatus() {
    ConsoleUtils::clearScreen();
    ConsoleUtils::printHeader("재고 현황");

    printf("%-4s %-16s %-9s %-9s %-9s %s\n", "ID", "이름", "실재재고", "필요재고", "가용재고", "상태");
    ConsoleUtils::printSeparator();
    for (auto* s : sample_svc_.getAllSamples()) {
        printf("%-4d %-16s %-9d %-9d %-9d %s\n",
               s->getId(), s->getName().c_str(),
               inventory_.getActualStock(s->getId()),
               inventory_.getRequiredStock(s->getId()),
               inventory_.getAvailableStock(s->getId()),
               inventory_.getStockStatus(s->getId()));
    }
    ConsoleUtils::pause();
}
