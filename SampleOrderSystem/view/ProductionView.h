// view/ProductionView.h
#pragma once
#include "service/ProductionLine.h"

class ProductionView {
public:
    explicit ProductionView(ProductionLine& production_line);
    void run();

private:
    ProductionLine& production_line_;
};
