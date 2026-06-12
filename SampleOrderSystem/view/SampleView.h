// view/SampleView.h
#pragma once
#include "service/SampleService.h"
#include "service/InventoryService.h"

class SampleView {
public:
    SampleView(SampleService& sample_svc, InventoryService& inventory);
    void run();

private:
    SampleService& sample_svc_;
    InventoryService& inventory_;
    void registerSample();
    void listSamples();
    void searchSample();
};
