// view/ConsoleUtils.cpp
#include "view/ConsoleUtils.h"
#include <iostream>
#include <limits>

void ConsoleUtils::clearScreen() { system("cls"); }

void ConsoleUtils::printHeader(const std::string& title) {
    printSeparator();
    std::cout << "  " << title << "\n";
    printSeparator();
}

void ConsoleUtils::printSeparator() {
    std::cout << "========================================\n";
}

int ConsoleUtils::readInt(const std::string& prompt) {
    int val;
    while (true) {
        std::cout << prompt;
        if (std::cin >> val) { std::cin.ignore(); return val; }
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "숫자를 입력하세요.\n";
    }
}

std::string ConsoleUtils::readString(const std::string& prompt) {
    std::string val;
    std::cout << prompt;
    std::getline(std::cin, val);
    return val;
}

void ConsoleUtils::pause() {
    std::cout << "\n계속하려면 Enter를 누르세요...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}
