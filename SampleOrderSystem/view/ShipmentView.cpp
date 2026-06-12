// view/ShipmentView.cpp
#include "view/ShipmentView.h"
#include "view/ConsoleUtils.h"
#include <iostream>
#include <cstdio>

ShipmentView::ShipmentView(ShipmentService& shipment_svc)
    : shipment_svc_(shipment_svc) {}

void ShipmentView::run() {
    ConsoleUtils::clearScreen();
    ConsoleUtils::printHeader("출고 처리");
    auto confirmed = shipment_svc_.getConfirmedOrders();
    if (confirmed.empty()) {
        std::cout << "출고 가능한 주문이 없습니다.\n";
        ConsoleUtils::pause(); return;
    }
    printf("%-4s %-16s %-7s %s\n", "ID", "고객명", "시료ID", "수량");
    ConsoleUtils::printSeparator();
    for (auto* o : confirmed)
        printf("%-4d %-16s %-7d %d\n", o->getId(), o->getCustomerName().c_str(),
               o->getSampleId(), o->getQuantity());
    int order_id = ConsoleUtils::readInt("\n출고할 주문 ID (0: 취소): ");
    if (order_id == 0) return;
    shipment_svc_.release(order_id)
        ? std::cout << "출고 처리 완료. 상태: RELEASE\n"
        : std::cout << "처리 실패. CONFIRMED 상태인지 확인하세요.\n";
    ConsoleUtils::pause();
}
