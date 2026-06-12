// view/ProductionView.cpp
#include "view/ProductionView.h"
#include "view/ConsoleUtils.h"
#include <iostream>
#include <cstdio>
#include <chrono>
#include <ctime>

ProductionView::ProductionView(ProductionLine& production_line,
                                SampleService& sample_svc, OrderService& order_svc)
    : production_line_(production_line), sample_svc_(sample_svc), order_svc_(order_svc) {}

void ProductionView::run() {
    production_line_.tick();
    ConsoleUtils::clearScreen();
    ConsoleUtils::printHeader("생산 라인");

    if (production_line_.isProducing()) {
        auto* job = production_line_.getCurrentJob();
        auto* sample = sample_svc_.findById(job->getSampleId());
        std::string sample_name = sample ? sample->getName() : "?";
        std::cout << "[현재 생산 중]\n";
        printf("  주문 ID:%d | 시료:%s | 생산량:%dea / %dea\n\n",
               job->getOrderId(), sample_name.c_str(),
               job->getProducedQty(), job->getActualQty());
    } else {
        std::cout << "[생산 중인 작업 없음]\n\n";
    }

    std::cout << "[대기 큐] " << production_line_.getQueueSize() << "건\n";

    auto q = production_line_.getWaitingQueue();
    if (q.empty()) {
        ConsoleUtils::pause();
        return;
    }

    std::cout << "\n";
    printf("%-4s %-8s %-16s %-8s %-8s %-8s %s\n",
           "순서", "주문ID", "시료 이름", "주문량", "부족분", "실생산량", "예상완료");
    ConsoleUtils::printSeparator();

    auto now = std::chrono::system_clock::now();

    double accumulated = 0.0;
    if (production_line_.isProducing()) {
        auto* cur = production_line_.getCurrentJob();
        int remaining = cur->getActualQty() - cur->getProducedQty();
        accumulated = remaining * cur->getAvgProductionTime();
    }

    int seq = 1;
    while (!q.empty()) {
        const ProductionJob& job = q.front();

        auto* sample = sample_svc_.findById(job.getSampleId());
        std::string sample_name = sample ? sample->getName() : "?";

        int order_qty = 0;
        for (auto* o : order_svc_.getAllOrders()) {
            if (o->getId() == job.getOrderId()) { order_qty = o->getQuantity(); break; }
        }

        accumulated += job.getActualQty() * job.getAvgProductionTime();
        auto completion_tp = now + std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::duration<double>(accumulated));
        std::time_t completion_t = std::chrono::system_clock::to_time_t(completion_tp);
        struct tm tm_buf{};
#if defined(_WIN32)
        localtime_s(&tm_buf, &completion_t);
#else
        localtime_r(&completion_t, &tm_buf);
#endif
        char time_str[9];
        strftime(time_str, sizeof(time_str), "%H:%M:%S", &tm_buf);

        char qty_str[12], short_str[12], actual_str[12];
        snprintf(qty_str,    sizeof(qty_str),    "%dea", order_qty);
        snprintf(short_str,  sizeof(short_str),  "%dea", job.getShortfall());
        snprintf(actual_str, sizeof(actual_str), "%dea", job.getActualQty());

        printf("%-4d %-8d %-16s %-8s %-8s %-8s %s\n",
               seq++, job.getOrderId(), sample_name.c_str(),
               qty_str, short_str, actual_str, time_str);

        q.pop();
    }

    ConsoleUtils::pause();
}
