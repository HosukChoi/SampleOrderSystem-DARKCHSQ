// test/JsonOrderRepositoryTest.cpp
#include <gtest/gtest.h>
#include "repository/JsonOrderRepository.h"
#include <filesystem>

namespace fs = std::filesystem;

class JsonOrderRepositoryTest : public ::testing::Test {
protected:
    const std::string file = "test_orders_tmp.json";
    JsonOrderRepository* repo;

    void SetUp() override {
        repo = new JsonOrderRepository(file);
    }
    void TearDown() override {
        delete repo;
        fs::remove(file);
    }
};

TEST_F(JsonOrderRepositoryTest, FindByIdReturnsNullWhenEmpty) {
    EXPECT_EQ(repo->findById(1), nullptr);
}

TEST_F(JsonOrderRepositoryTest, SaveAndFindById) {
    Order o(1, 10, "Lab-A", 5);
    repo->save(o);
    Order* found = repo->findById(1);
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->getSampleId(), 10);
    EXPECT_EQ(found->getCustomerName(), "Lab-A");
    EXPECT_EQ(found->getQuantity(), 5);
    EXPECT_EQ(found->getStatus(), OrderStatus::RESERVED);
}

TEST_F(JsonOrderRepositoryTest, FindAllReturnsAllSaved) {
    repo->save(Order(1, 10, "Lab-A", 5));
    repo->save(Order(2, 20, "Lab-B", 3));
    EXPECT_EQ(repo->findAll().size(), 2u);
}

TEST_F(JsonOrderRepositoryTest, FindByStatusFiltersCorrectly) {
    Order o1(1, 10, "Lab-A", 5); o1.setStatus(OrderStatus::CONFIRMED);
    Order o2(2, 10, "Lab-B", 3); o2.setStatus(OrderStatus::PRODUCING);
    Order o3(3, 10, "Lab-C", 2);
    repo->save(o1); repo->save(o2); repo->save(o3);

    EXPECT_EQ(repo->findByStatus(OrderStatus::CONFIRMED).size(), 1u);
    EXPECT_EQ(repo->findByStatus(OrderStatus::PRODUCING).size(), 1u);
    EXPECT_EQ(repo->findByStatus(OrderStatus::RESERVED).size(), 1u);
}

TEST_F(JsonOrderRepositoryTest, NextIdIncrements) {
    EXPECT_EQ(repo->nextId(), 1);
    EXPECT_EQ(repo->nextId(), 2);
    EXPECT_EQ(repo->nextId(), 3);
}

TEST_F(JsonOrderRepositoryTest, PersistAndReloadRoundTrip) {
    Order o(1, 10, "Lab-A", 5); o.setStatus(OrderStatus::CONFIRMED);
    repo->save(o);
    delete repo;

    repo = new JsonOrderRepository(file);
    Order* found = repo->findById(1);
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->getStatus(), OrderStatus::CONFIRMED);
    EXPECT_EQ(found->getCustomerName(), "Lab-A");
}

TEST_F(JsonOrderRepositoryTest, SaveUpdatesExistingOrder) {
    Order o(1, 10, "Lab-A", 5);
    repo->save(o);
    o.setStatus(OrderStatus::CONFIRMED);
    repo->save(o);
    EXPECT_EQ(repo->findAll().size(), 1u);
    EXPECT_EQ(repo->findById(1)->getStatus(), OrderStatus::CONFIRMED);
}

TEST_F(JsonOrderRepositoryTest, PersistIntegrity_StatusRoundTrip) {
    Order o1(1, 10, "Lab-A", 5); o1.setStatus(OrderStatus::PRODUCING); repo->save(o1);
    Order o2(2, 10, "Lab-B", 3); o2.setStatus(OrderStatus::RELEASE);   repo->save(o2);
    Order o3(3, 10, "Lab-C", 2); o3.setStatus(OrderStatus::REJECTED);  repo->save(o3);
    delete repo;

    repo = new JsonOrderRepository(file);
    EXPECT_EQ(repo->findById(1)->getStatus(), OrderStatus::PRODUCING);
    EXPECT_EQ(repo->findById(2)->getStatus(), OrderStatus::RELEASE);
    EXPECT_EQ(repo->findById(3)->getStatus(), OrderStatus::REJECTED);
}

TEST_F(JsonOrderRepositoryTest, NextIdPersistsAcrossRestart) {
    repo->save(Order(repo->nextId(), 10, "Lab-A", 1)); // id=1
    repo->save(Order(repo->nextId(), 10, "Lab-B", 1)); // id=2
    delete repo;

    repo = new JsonOrderRepository(file);
    EXPECT_EQ(repo->nextId(), 3);
}

TEST_F(JsonOrderRepositoryTest, FindByStatusEmptyResult) {
    repo->save(Order(1, 10, "Lab-A", 5)); // RESERVED
    EXPECT_TRUE(repo->findByStatus(OrderStatus::CONFIRMED).empty());
    EXPECT_TRUE(repo->findByStatus(OrderStatus::PRODUCING).empty());
}
