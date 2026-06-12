// interface/IClockProvider.h
#pragma once
#include <chrono>

#define interface struct

interface IClockProvider {
    virtual ~IClockProvider() = default;
    virtual std::chrono::system_clock::time_point now() const = 0;
};
