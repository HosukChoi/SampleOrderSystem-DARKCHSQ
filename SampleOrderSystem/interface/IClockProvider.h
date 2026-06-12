// interface/IClockProvider.h
#pragma once
#include <chrono>

#ifndef interface
#define interface struct
#endif

interface IClockProvider {
    virtual ~IClockProvider() = default;
    virtual std::chrono::system_clock::time_point now() const = 0;
};
