// test/InventoryServiceTest.cpp
#include <gtest/gtest.h>
#include "service/InventoryService.h"
#include "repository/JsonOrderRepository.h"
#include <filesystem>

namespace fs = std::filesystem;

class InventoryServiceTest : public ::testing::Test {
protected:
    const std::string order_file = "test_inv_orders_tmp.json";
    const std::string inv_file   = "test_inv_tmp.json";
    JsonOrderRepository* order_repo;
    InventoryService* inv;

    void SetUp() override {
        order_repo = new JsonOrderRepository(order_file);
        inv        = new InventoryService(*order_repo, inv_file);
    }
    void TearDown() override {
        delete inv;
        delete order_repo;
        fs::remove(order_file);
        fs::remove(inv_file);
    }
};

TEST_F(InventoryServiceTest, ActualStockStartsAtZero) {
    EXPECT_EQ(inv->getActualStock(1), 0);
}

TEST_F(InventoryServiceTest, AddActualStockIncrements) {
    inv->addActualStock(1, 50);
    EXPECT_EQ(inv->getActualStock(1), 50);
    inv->addActualStock(1, 30);
    EXPECT_EQ(inv->getActualStock(1), 80);
}

TEST_F(InventoryServiceTest, SubtractActualStockDecrements) {
    inv->addActualStock(1, 50);
    inv->subtractActualStock(1, 20);
    EXPECT_EQ(inv->getActualStock(1), 30);
}

TEST_F(InventoryServiceTest, RequiredStockCountsConfirmedAndProducing) {
    Order o1(1, 1, "Lab-A", 30); o1.setStatus(OrderStatus::CONFIRMED);
    Order o2(2, 1, "Lab-B", 20); o2.setStatus(OrderStatus::PRODUCING);
    Order o3(3, 1, "Lab-C", 10); o3.setStatus(OrderStatus::RESERVED);
    order_repo->save(o1);
    order_repo->save(o2);
    order_repo->save(o3);
    EXPECT_EQ(inv->getRequiredStock(1), 50);  // RESERVED 제외
}

TEST_F(InventoryServiceTest, AvailableStockIsActualMinusRequired) {
    inv->addActualStock(1, 100);
    Order o(1, 1, "Lab-A", 30); o.setStatus(OrderStatus::CONFIRMED);
    order_repo->save(o);
    EXPECT_EQ(inv->getAvailableStock(1), 70);
}

TEST_F(InventoryServiceTest, StockStatusSufficient) {
    inv->addActualStock(1, 100);
    Order o(1, 1, "Lab-A", 30); o.setStatus(OrderStatus::CONFIRMED);
    order_repo->save(o);
    EXPECT_STREQ(inv->getStockStatus(1), "여유");
}

TEST_F(InventoryServiceTest, StockStatusShortage) {
    inv->addActualStock(1, 10);
    Order o(1, 1, "Lab-A", 30); o.setStatus(OrderStatus::CONFIRMED);
    order_repo->save(o);
    EXPECT_STREQ(inv->getStockStatus(1), "부족");
}

TEST_F(InventoryServiceTest, StockStatusDepleted) {
    EXPECT_STREQ(inv->getStockStatus(1), "고갈");
}

TEST_F(InventoryServiceTest, PersistenceRoundTrip) {
    inv->addActualStock(1, 77);
    JsonOrderRepository order2(order_file);
    InventoryService inv2(order2, inv_file);
    EXPECT_EQ(inv2.getActualStock(1), 77);
}
