// repository/JsonSampleRepository.cpp
#include "repository/JsonSampleRepository.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>

using json = nlohmann::json;
namespace fs = std::filesystem;

JsonSampleRepository::JsonSampleRepository(const std::string& filePath)
    : file_path_(filePath) {
    load();
}

void JsonSampleRepository::load() {
    std::ifstream f(file_path_);
    if (!f.is_open()) return;
    json j; f >> j;
    for (auto& item : j)
        store_.emplace(item["id"].get<int>(),
            Sample(item["id"].get<int>(),
                   item["name"].get<std::string>(),
                   item["avg_production_time"].get<double>(),
                   item["yield"].get<double>()));
}

void JsonSampleRepository::persist() const {
    auto parent = fs::path(file_path_).parent_path();
    if (!parent.empty()) fs::create_directories(parent);
    json j = json::array();
    for (auto& [id, s] : store_)
        j.push_back({{"id",                   s.getId()},
                     {"name",                 s.getName()},
                     {"avg_production_time",  s.getAvgProductionTime()},
                     {"yield",                s.getYield()}});
    std::ofstream(file_path_) << j.dump(2);
}

void JsonSampleRepository::save(const Sample& sample) {
    store_.emplace(sample.getId(), sample);
    persist();
}

Sample* JsonSampleRepository::findById(int id) {
    auto it = store_.find(id);
    return it != store_.end() ? &it->second : nullptr;
}

std::vector<Sample*> JsonSampleRepository::findAll() {
    std::vector<Sample*> result;
    for (auto& [id, sample] : store_)
        result.push_back(&sample);
    return result;
}

std::vector<Sample*> JsonSampleRepository::findByName(const std::string& keyword) {
    std::vector<Sample*> result;
    for (auto& [id, sample] : store_)
        if (sample.getName().find(keyword) != std::string::npos)
            result.push_back(&sample);
    return result;
}
