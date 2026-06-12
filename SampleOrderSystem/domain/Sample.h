// domain/Sample.h
#pragma once
#include <string>

class Sample {
public:
    Sample(int id, const std::string& name, double avg_production_time, double yield);

    int getId() const;
    const std::string& getName() const;
    double getAvgProductionTime() const;
    double getYield() const;

private:
    int id_;
    std::string name_;
    double avg_production_time_;
    double yield_;
};
