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
    production_line_.tick();
    ConsoleUtils::clearScreen();
    ConsoleUtils::printHeader("모니터링");

    auto all = order_svc_.getAllOrders();
    int cnt[4] = {};
    for (auto* o : all) {
        if (o->getStatus() == OrderStatus::RESERVED)  cnt[0]++;
        if (o->getStatus() == OrderStatus::PRODUCING) cnt[1]++;
        if (o->getStatus() == OrderStatus::CONFIRMED) cnt[2]++;
        if (o->getStatus() == OrderStatus::RELEASE)   cnt[3]++;
    }
    std::cout << "[주문 현황]\n";
    printf("  RESERVED:%d  PRODUCING:%d  CONFIRMED:%d  RELEASE:%d\n\n",
           cnt[0], cnt[1], cnt[2], cnt[3]);

    if (production_line_.isProducing()) {
        auto* job = production_line_.getCurrentJob();
        std::cout << "[현재 생산 중]\n";
        printf("  주문 ID:%d | 시료 ID:%d | 생산량:%d / %d\n\n",
               job->getOrderId(), job->getSampleId(),
               job->getProducedQty(), job->getActualQty());
    }

    std::cout << "[재고 현황]\n";
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
