// view/MonitoringView.cpp
#include "view/MonitoringView.h"
#include "view/ConsoleUtils.h"
#include <iostream>
#include <cstdio>
#include <chrono>
#include <ctime>
#include <queue>

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

    showProductionQueue();
    ConsoleUtils::pause();
}

void MonitoringView::showProductionQueue() {
    std::cout << "\n";
    ConsoleUtils::printHeader("생산 큐 현황");
    printf("%-4s %-8s %-16s %-8s %-8s %-8s %s\n",
           "순서", "주문ID", "시료 이름", "주문량", "부족분", "실생산량", "예상완료");
    ConsoleUtils::printSeparator();

    if (!production_line_.isProducing()) {
        std::cout << "  (생산 중인 작업 없음)\n";
        return;
    }

    auto now = std::chrono::system_clock::now();

    auto printJob = [&](int seq, const ProductionJob& job, double seconds_from_now) {
        auto* sample = sample_svc_.findById(job.getSampleId());
        std::string sample_name = sample ? sample->getName() : "?";

        int order_qty = 0;
        for (auto* o : order_svc_.getAllOrders()) {
            if (o->getId() == job.getOrderId()) { order_qty = o->getQuantity(); break; }
        }

        auto completion_tp = now + std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::duration<double>(seconds_from_now));
        std::time_t completion_t = std::chrono::system_clock::to_time_t(completion_tp);
        struct tm tm_buf{};
#if defined(_WIN32)
        localtime_s(&tm_buf, &completion_t);
#else
        localtime_r(&completion_t, &tm_buf);
#endif
        char time_str[9];
        strftime(time_str, sizeof(time_str), "%H:%M:%S", &tm_buf);

        printf("%-4d %-8d %-16s %-8d %-8d %-8d %s\n",
               seq, job.getOrderId(), sample_name.c_str(),
               order_qty, job.getShortfall(), job.getActualQty(), time_str);
    };

    const ProductionJob* cur = production_line_.getCurrentJob();

    int remaining_cur = cur->getActualQty() - cur->getProducedQty();
    double secs_cur = remaining_cur * cur->getAvgProductionTime();
    printJob(1, *cur, secs_cur);

    double accumulated = secs_cur;
    auto q = production_line_.getWaitingQueue();
    int seq = 2;
    while (!q.empty()) {
        const ProductionJob& job = q.front();
        accumulated += job.getActualQty() * job.getAvgProductionTime();
        printJob(seq++, job, accumulated);
        q.pop();
    }
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
