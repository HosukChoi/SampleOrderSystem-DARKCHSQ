// view/ShipmentView.cpp
#include "view/ShipmentView.h"
#include "view/ConsoleUtils.h"
#include <iostream>
#include <cstdio>

ShipmentView::ShipmentView(ShipmentService& shipment_svc, SampleService& sample_svc)
    : shipment_svc_(shipment_svc), sample_svc_(sample_svc) {}

void ShipmentView::run() {
    ConsoleUtils::clearScreen();
    ConsoleUtils::printHeader("출고 처리");
    auto confirmed = shipment_svc_.getConfirmedOrders();
    if (confirmed.empty()) {
        std::cout << "출고 가능한 주문이 없습니다.\n";
        ConsoleUtils::pause(); return;
    }
    printf("%-4s %-8s %-16s %-16s %s\n", "No", "주문ID", "고객명", "시료 이름", "수량");
    ConsoleUtils::printSeparator();
    int no = 1;
    for (auto* o : confirmed) {
        auto* s = sample_svc_.findById(o->getSampleId());
        std::string sample_name = s ? s->getName() : "?";
        printf("%-4d %-8d %-16s %-16s %d\n",
               no++, o->getId(), o->getCustomerName().c_str(),
               sample_name.c_str(), o->getQuantity());
    }
    int order_id = ConsoleUtils::readInt("\n출고할 주문 ID (0: 취소): ");
    if (order_id == 0) return;
    shipment_svc_.release(order_id)
        ? std::cout << "출고 처리 완료. 상태: RELEASE\n"
        : std::cout << "처리 실패. CONFIRMED 상태인지 확인하세요.\n";
    ConsoleUtils::pause();
}
