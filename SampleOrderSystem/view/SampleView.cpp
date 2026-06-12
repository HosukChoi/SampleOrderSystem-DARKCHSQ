// view/SampleView.cpp
#include "view/SampleView.h"
#include "view/ConsoleUtils.h"
#include <iostream>
#include <cstdio>

SampleView::SampleView(SampleService& sample_svc, InventoryService& inventory)
    : sample_svc_(sample_svc), inventory_(inventory) {}

void SampleView::run() {
    while (true) {
        ConsoleUtils::clearScreen();
        ConsoleUtils::printHeader("시료 관리");
        std::cout << "1. 시료 등록\n2. 시료 조회\n3. 시료 검색\n0. 돌아가기\n";
        int choice = ConsoleUtils::readInt("> ");
        if (choice == 0) return;
        if (choice == 1) registerSample();
        else if (choice == 2) listSamples();
        else if (choice == 3) searchSample();
    }
}

void SampleView::registerSample() {
    ConsoleUtils::printHeader("시료 등록");
    int id = ConsoleUtils::readInt("시료 ID: ");
    std::string name = ConsoleUtils::readString("시료 이름: ");
    int avg_time = ConsoleUtils::readInt("평균 생산시간(초): ");
    int yield_pct = ConsoleUtils::readInt("수율(%): ");

    if (sample_svc_.registerSample(id, name, static_cast<double>(avg_time), yield_pct / 100.0))
        std::cout << "시료가 등록되었습니다.\n";
    else
        std::cout << "이미 존재하는 ID입니다.\n";
    ConsoleUtils::pause();
}

void SampleView::listSamples() {
    ConsoleUtils::printHeader("시료 목록");
    auto samples = sample_svc_.getAllSamples();
    if (samples.empty()) { std::cout << "등록된 시료가 없습니다.\n"; }
    else {
        printf("%-4s %-16s %-12s %-8s %-10s %s\n", "ID", "이름", "평균생산시간", "수율", "현재 재고", "상태");
        ConsoleUtils::printSeparator();
        for (auto* s : samples) {
            char yield_str[16], stock_str[16];
            snprintf(yield_str, sizeof(yield_str), "%.0f%%", s->getYield() * 100);
            snprintf(stock_str, sizeof(stock_str), "%dea", inventory_.getActualStock(s->getId()));
            printf("%-4d %-16s %-12.1f %-8s %-10s %s\n",
                   s->getId(), s->getName().c_str(),
                   s->getAvgProductionTime(), yield_str,
                   stock_str, inventory_.getStockStatus(s->getId()));
        }
    }
    ConsoleUtils::pause();
}

void SampleView::searchSample() {
    ConsoleUtils::printHeader("시료 검색");
    std::string keyword = ConsoleUtils::readString("검색어: ");
    auto results = sample_svc_.searchByName(keyword);
    if (results.empty()) std::cout << "검색 결과 없음.\n";
    else
        for (auto* s : results)
            printf("ID:%-4d 이름:%-16s 수율:%.0f%%\n",
                   s->getId(), s->getName().c_str(), s->getYield() * 100);
    ConsoleUtils::pause();
}
