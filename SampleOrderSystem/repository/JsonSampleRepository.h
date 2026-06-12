// repository/JsonSampleRepository.h
#pragma once
#include <map>
#include <string>
#include "interface/ISampleRepository.h"

class JsonSampleRepository : public ISampleRepository {
public:
    explicit JsonSampleRepository(const std::string& filePath);

    int getId() const override { return 1; }
    void save(const Sample& sample) override;
    Sample* findById(int id) override;
    std::vector<Sample*> findAll() override;
    std::vector<Sample*> findByName(const std::string& keyword) override;

private:
    std::string       file_path_;
    std::map<int, Sample> store_;

    void load();
    void persist() const;
};
