// domain/Sample.cpp
#include "domain/Sample.h"

Sample::Sample(int id, const std::string& name, double avg_production_time, double yield)
    : id_(id), name_(name), avg_production_time_(avg_production_time), yield_(yield) {}

int Sample::getId() const { return id_; }
const std::string& Sample::getName() const { return name_; }
double Sample::getAvgProductionTime() const { return avg_production_time_; }
double Sample::getYield() const { return yield_; }
