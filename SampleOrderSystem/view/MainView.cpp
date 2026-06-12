// view/MainView.cpp
#include "view/MainView.h"
#include "view/ConsoleUtils.h"
#include <iostream>
#include <cstdio>
#include <chrono>
#include <ctime>

MainView::MainView(SampleService& sample_svc, OrderService& order_svc,
                   InventoryService& inventory, ProductionLine& production_line,
                   ShipmentService& shipment_svc)
    : sample_svc_(sample_svc), order_svc_(order_svc), inventory_(inventory),
      production_line_(production_line), shipment_svc_(shipment_svc) {}

void MainView::printSummary() {
    auto now_t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    struct tm tm_buf{};
#if defined(_WIN32)
    localtime_s(&tm_buf, &now_t);
#else
    localtime_r(&now_t, &tm_buf);
#endif
    char time_str[9];
    strftime(time_str, sizeof(time_str), "%H:%M:%S", &tm_buf);

    int total_stock = 0;
    for (auto* s : sample_svc_.getAllSamples())
        total_stock += inventory_.getActualStock(s->getId());

    int queue_size = production_line_.getQueueSize();
    std::string queue_label = production_line_.isProducing()
        ? (std::to_string(queue_size) + "건 대기 (생산 중)")
        : "0건 대기";

    printf("현재 시각: %s\n", time_str);
    printf("등록 시료: %d종  |  전체 주문: %d건  |  총 재고: %dea  |  생산 큐: %s\n",
           (int)sample_svc_.getAllSamples().size(),
           (int)order_svc_.getAllOrders().size(),
           total_stock,
           queue_label.c_str());
}

int MainView::promptMainMenu() {
    ConsoleUtils::clearScreen();
    ConsoleUtils::printHeader("S-Semi 시료 생산주문관리 시스템");
    printSummary();
    std::cout << "\n1. 시료 관리\n2. 시료 주문\n3. 주문 승인/거절\n"
                 "4. 모니터링\n5. 생산라인 조회\n6. 출고 처리\n0. 종료\n";
    return ConsoleUtils::readInt("> ");
}
