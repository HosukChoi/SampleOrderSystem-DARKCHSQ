// view/ConsoleUtils.h
#pragma once
#include <string>

class ConsoleUtils {
public:
    static void clearScreen();
    static void printHeader(const std::string& title);
    static void printSeparator();
    static int readInt(const std::string& prompt);
    static std::string readString(const std::string& prompt);
    static void pause();
};
