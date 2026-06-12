// test/JsonSampleRepositoryTest.cpp
#include <gtest/gtest.h>
#include "repository/JsonSampleRepository.h"
#include <filesystem>

namespace fs = std::filesystem;

class JsonSampleRepositoryTest : public ::testing::Test {
protected:
    const std::string file = "test_samples_tmp.json";
    JsonSampleRepository* repo;

    void SetUp() override {
        repo = new JsonSampleRepository(file);
    }
    void TearDown() override {
        delete repo;
        fs::remove(file);
    }
};

TEST_F(JsonSampleRepositoryTest, FindByIdReturnsNullWhenEmpty) {
    EXPECT_EQ(repo->findById(1), nullptr);
}

TEST_F(JsonSampleRepositoryTest, SaveAndFindById) {
    repo->save(Sample(1, "AlphaSi", 3.0, 0.9));
    Sample* s = repo->findById(1);
    ASSERT_NE(s, nullptr);
    EXPECT_EQ(s->getName(), "AlphaSi");
    EXPECT_DOUBLE_EQ(s->getAvgProductionTime(), 3.0);
    EXPECT_DOUBLE_EQ(s->getYield(), 0.9);
}

TEST_F(JsonSampleRepositoryTest, FindAllReturnsAllSaved) {
    repo->save(Sample(1, "AlphaSi", 3.0, 0.9));
    repo->save(Sample(2, "BetaSi",  5.0, 0.85));
    EXPECT_EQ(repo->findAll().size(), 2u);
}

TEST_F(JsonSampleRepositoryTest, FindByNameKeywordMatch) {
    repo->save(Sample(1, "AlphaSi", 3.0, 0.9));
    repo->save(Sample(2, "BetaSi",  5.0, 0.85));
    repo->save(Sample(3, "GammaSi", 4.0, 0.88));
    auto result = repo->findByName("Si");
    EXPECT_EQ(result.size(), 3u);
    auto partial = repo->findByName("Alpha");
    ASSERT_EQ(partial.size(), 1u);
    EXPECT_EQ(partial[0]->getName(), "AlphaSi");
}

TEST_F(JsonSampleRepositoryTest, PersistAndReloadRoundTrip) {
    repo->save(Sample(1, "AlphaSi", 3.0, 0.9));
    repo->save(Sample(2, "BetaSi",  5.0, 0.85));
    delete repo;

    repo = new JsonSampleRepository(file);
    ASSERT_NE(repo->findById(1), nullptr);
    ASSERT_NE(repo->findById(2), nullptr);
    EXPECT_EQ(repo->findById(1)->getName(), "AlphaSi");
    EXPECT_EQ(repo->findById(2)->getName(), "BetaSi");
}

TEST_F(JsonSampleRepositoryTest, SaveIgnoresDuplicateId) {
    repo->save(Sample(1, "AlphaSi",    3.0, 0.9));
    repo->save(Sample(1, "AlphaSi_v2", 2.0, 0.8));
    EXPECT_EQ(repo->findAll().size(), 1u);
    EXPECT_EQ(repo->findById(1)->getName(), "AlphaSi");  // 최초 저장값 유지
}

TEST_F(JsonSampleRepositoryTest, FindByNameNoMatch) {
    repo->save(Sample(1, "AlphaSi", 3.0, 0.9));
    EXPECT_TRUE(repo->findByName("ZZ_NoMatch").empty());
}

TEST_F(JsonSampleRepositoryTest, PersistIntegrity_AllFields) {
    repo->save(Sample(5, "GammaSi", 7.5, 0.75));
    delete repo;
    repo = new JsonSampleRepository(file);
    Sample* s = repo->findById(5);
    ASSERT_NE(s, nullptr);
    EXPECT_EQ(s->getId(), 5);
    EXPECT_EQ(s->getName(), "GammaSi");
    EXPECT_DOUBLE_EQ(s->getAvgProductionTime(), 7.5);
    EXPECT_DOUBLE_EQ(s->getYield(), 0.75);
}
