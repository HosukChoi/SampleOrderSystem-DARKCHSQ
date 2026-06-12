// service/SampleService.cpp
#include "service/SampleService.h"

SampleService::SampleService(ISampleRepository& sample_repo)
    : sample_repo_(sample_repo) {}

bool SampleService::registerSample(int id, const std::string& name,
                                   double avg_production_time, double yield) {
    if (sample_repo_.findById(id) != nullptr) return false;
    sample_repo_.save(Sample(id, name, avg_production_time, yield));
    return true;
}

std::vector<Sample*> SampleService::getAllSamples() {
    return sample_repo_.findAll();
}

Sample* SampleService::findById(int id) {
    return sample_repo_.findById(id);
}

std::vector<Sample*> SampleService::searchByName(const std::string& keyword) {
    return sample_repo_.findByName(keyword);
}
