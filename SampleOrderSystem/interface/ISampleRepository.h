// interface/ISampleRepository.h
#pragma once
#include <vector>
#include <string>
#include "interface/IComponent.h"
#include "domain/Sample.h"

interface ISampleRepository : public IComponent {
    virtual void save(const Sample& sample) = 0;
    virtual Sample* findById(int id) = 0;
    virtual std::vector<Sample*> findAll() = 0;
    virtual std::vector<Sample*> findByName(const std::string& keyword) = 0;
    virtual int getId() const override = 0;
};
