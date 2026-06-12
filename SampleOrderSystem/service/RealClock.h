// service/RealClock.h
#pragma once
#include "interface/IClockProvider.h"

class RealClock : public IClockProvider {
public:
    std::chrono::system_clock::time_point now() const override {
        return std::chrono::system_clock::now();
    }
};
