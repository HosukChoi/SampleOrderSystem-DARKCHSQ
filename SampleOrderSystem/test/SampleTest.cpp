// test/SampleTest.cpp
#include <gtest/gtest.h>
#include "domain/Sample.h"

TEST(SampleTest, StoresFieldsCorrectly) {
    Sample s(1, "AlphaSi", 3.0, 0.9);
    EXPECT_EQ(s.getId(), 1);
    EXPECT_EQ(s.getName(), "AlphaSi");
    EXPECT_DOUBLE_EQ(s.getAvgProductionTime(), 3.0);
    EXPECT_DOUBLE_EQ(s.getYield(), 0.9);
}

TEST(SampleTest, YieldBoundary_One) {
    Sample s(2, "PerfectSi", 1.0, 1.0);
    EXPECT_DOUBLE_EQ(s.getYield(), 1.0);
}

TEST(SampleTest, YieldBoundary_Low) {
    Sample s(3, "LowYieldSi", 2.0, 0.1);
    EXPECT_DOUBLE_EQ(s.getYield(), 0.1);
}
