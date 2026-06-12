// view/ConsoleUtils.cpp
#include "view/ConsoleUtils.h"
#include <iostream>
#include <limits>
#include <cstdint>

namespace {
int displayWidth(const std::string& s) {
    int width = 0;
    size_t i = 0;
    while (i < s.size()) {
        unsigned char c = (unsigned char)s[i];
        uint32_t cp = 0;
        size_t bytes;
        if      (c < 0x80) { cp = c; bytes = 1; }
        else if (c < 0xE0) { cp = c & 0x1F; bytes = 2; }
        else if (c < 0xF0) { cp = c & 0x0F; bytes = 3; }
        else               { cp = c & 0x07; bytes = 4; }
        for (size_t j = 1; j < bytes; j++)
            if (i + j < s.size())
                cp = (cp << 6) | ((unsigned char)s[i + j] & 0x3F);
        i += bytes;
        if ((cp >= 0x1100 && cp <= 0x11FF) ||
            (cp >= 0x2E80 && cp <= 0x9FFF) ||
            (cp >= 0xAC00 && cp <= 0xD7A3) ||
            (cp >= 0xF900 && cp <= 0xFAFF) ||
            (cp >= 0xFF01 && cp <= 0xFF60))
            width += 2;
        else
            width += 1;
    }
    return width;
}
} // namespace

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

std::string ConsoleUtils::pad(const std::string& s, int col_width) {
    int spaces = col_width - displayWidth(s);
    if (spaces <= 0) return s;
    return s + std::string(spaces, ' ');
}

void ConsoleUtils::pause() {
    std::cout << "\n계속하려면 Enter를 누르세요...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}
