// service/SampleService.h
#pragma once
#include <vector>
#include <string>
#include "interface/ISampleRepository.h"

class SampleService {
public:
    explicit SampleService(ISampleRepository& sample_repo);

    bool registerSample(int id, const std::string& name,
                        double avg_production_time, double yield);
    std::vector<Sample*> getAllSamples();
    Sample* findById(int id);
    std::vector<Sample*> searchByName(const std::string& keyword);

private:
    ISampleRepository& sample_repo_;
};
