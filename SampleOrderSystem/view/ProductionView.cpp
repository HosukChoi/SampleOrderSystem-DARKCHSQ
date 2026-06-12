// view/ProductionView.cpp
#include "view/ProductionView.h"
#include "view/ConsoleUtils.h"
#include <iostream>
#include <cstdio>

ProductionView::ProductionView(ProductionLine& production_line)
    : production_line_(production_line) {}

void ProductionView::run() {
    production_line_.tick();
    ConsoleUtils::clearScreen();
    ConsoleUtils::printHeader("생산 라인");
    if (production_line_.isProducing()) {
        auto* job = production_line_.getCurrentJob();
        std::cout << "[현재 생산 중]\n";
        printf("  주문 ID:%d | 시료 ID:%d | 생산량:%dea / %dea\n\n",
               job->getOrderId(), job->getSampleId(),
               job->getProducedQty(), job->getActualQty());
    } else {
        std::cout << "[생산 중인 작업 없음]\n\n";
    }
    std::cout << "[대기 큐] " << production_line_.getQueueSize() << "건\n";
    auto q = production_line_.getWaitingQueue();
    int pos = 1;
    while (!q.empty()) {
        auto job = q.front(); q.pop();
        printf("  %d. 주문ID:%d | 시료ID:%d | 예정:%dea\n",
               pos++, job.getOrderId(), job.getSampleId(), job.getActualQty());
    }
    ConsoleUtils::pause();
}
