// view/MainView.cpp
#include "view/MainView.h"
#include "view/ConsoleUtils.h"
#include <iostream>

MainView::MainView(SampleService& sample_svc, OrderService& order_svc,
                   InventoryService& inventory, ProductionLine& production_line,
                   ShipmentService& shipment_svc)
    : sample_svc_(sample_svc), order_svc_(order_svc), inventory_(inventory),
      production_line_(production_line), shipment_svc_(shipment_svc) {}

void MainView::printSummary() {
    std::cout << "등록 시료: " << sample_svc_.getAllSamples().size() << "종  "
              << "전체 주문: " << order_svc_.getAllOrders().size() << "건";
    if (production_line_.isProducing()) std::cout << "  [생산 중]";
    std::cout << "\n";
}

int MainView::promptMainMenu() {
    ConsoleUtils::clearScreen();
    ConsoleUtils::printHeader("S-Semi 시료 생산주문관리 시스템");
    printSummary();
    std::cout << "\n1. 시료 관리\n2. 주문 (접수/승인/거절)\n"
                 "3. 모니터링\n4. 출고 처리\n5. 생산 라인\n0. 종료\n";
    return ConsoleUtils::readInt("> ");
}
