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
