// test/SampleServiceTest.cpp
#include <gtest/gtest.h>
#include "service/SampleService.h"
#include "repository/JsonSampleRepository.h"
#include <filesystem>

namespace fs = std::filesystem;

class SampleServiceTest : public ::testing::Test {
protected:
    const std::string sample_file = "test_samples_svc_tmp.json";
    JsonSampleRepository* repo;
    SampleService* svc;

    void SetUp() override {
        repo = new JsonSampleRepository(sample_file);
        svc  = new SampleService(*repo);
    }
    void TearDown() override {
        delete svc;
        delete repo;
        fs::remove(sample_file);
    }
};

TEST_F(SampleServiceTest, RegisterSampleSucceeds) {
    EXPECT_TRUE(svc->registerSample(1, "AlphaSi", 3.0, 0.9));
}

TEST_F(SampleServiceTest, RegisterDuplicateIdFails) {
    svc->registerSample(1, "AlphaSi", 3.0, 0.9);
    EXPECT_FALSE(svc->registerSample(1, "BetaSi", 2.0, 0.8));
}

TEST_F(SampleServiceTest, GetAllSamplesReturnsRegistered) {
    svc->registerSample(1, "AlphaSi", 3.0, 0.9);
    svc->registerSample(2, "BetaSi",  2.0, 0.8);
    EXPECT_EQ(svc->getAllSamples().size(), 2u);
}

TEST_F(SampleServiceTest, FindByIdReturnsCorrectSample) {
    svc->registerSample(1, "AlphaSi", 3.0, 0.9);
    auto* s = svc->findById(1);
    ASSERT_NE(s, nullptr);
    EXPECT_EQ(s->getName(), "AlphaSi");
}

TEST_F(SampleServiceTest, FindByIdReturnsNullForMissing) {
    EXPECT_EQ(svc->findById(99), nullptr);
}

TEST_F(SampleServiceTest, SearchByNameFindsMatch) {
    svc->registerSample(1, "AlphaSi", 3.0, 0.9);
    svc->registerSample(2, "BetaSi",  2.0, 0.8);
    auto results = svc->searchByName("Alpha");
    ASSERT_EQ(results.size(), 1u);
    EXPECT_EQ(results[0]->getId(), 1);
}
