// test/ProductionLineTest.cpp
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "service/ProductionLine.h"
#include "service/InventoryService.h"
#include "repository/JsonOrderRepository.h"
#include "interface/IClockProvider.h"
#include <filesystem>

namespace fs = std::filesystem;
using ::testing::Return;
using TimePoint = std::chrono::system_clock::time_point;

struct MockClock : public IClockProvider {
    MOCK_METHOD(TimePoint, now, (), (const, override));
};

inline TimePoint advanceSec(TimePoint base, double sec) {
    return base + std::chrono::duration_cast<std::chrono::system_clock::duration>(
        std::chrono::duration<double>(sec));
}

class ProductionLineTest : public ::testing::Test {
protected:
    const std::string order_file = "test_pl_orders_tmp.json";
    const std::string inv_file   = "test_pl_inv_tmp.json";
    JsonOrderRepository* order_repo;
    InventoryService*    inv;
    MockClock            mock_clock;
    ProductionLine*      prod;
    TimePoint            t0;

    void SetUp() override {
        t0         = std::chrono::system_clock::now();
        order_repo = new JsonOrderRepository(order_file);
        inv        = new InventoryService(*order_repo, inv_file);
        prod       = new ProductionLine(*inv, *order_repo, mock_clock);
    }
    void TearDown() override {
        delete prod;
        delete inv;
        delete order_repo;
        fs::remove(order_file);
        fs::remove(inv_file);
    }
};

TEST_F(ProductionLineTest, NotProducingInitially) {
    EXPECT_FALSE(prod->isProducing());
}

TEST_F(ProductionLineTest, EnqueueStartsProduction) {
    EXPECT_CALL(mock_clock, now()).WillRepeatedly(Return(t0));
    prod->enqueue(1, 1, 3, 1.0);
    EXPECT_TRUE(prod->isProducing());
}

TEST_F(ProductionLineTest, SecondEnqueueGoesToWaitingQueue) {
    EXPECT_CALL(mock_clock, now()).WillRepeatedly(Return(t0));
    prod->enqueue(1, 1, 3, 1.0);
    prod->enqueue(2, 1, 2, 1.0);
    EXPECT_EQ(prod->getQueueSize(), 1);
}

TEST_F(ProductionLineTest, TickUpdatesInventoryAfterElapsed) {
    EXPECT_CALL(mock_clock, now())
        .WillOnce(Return(t0))
        .WillOnce(Return(advanceSec(t0, 2.5)));

    prod->enqueue(1, 1, 2, 1.0);
    prod->tick();
    EXPECT_EQ(inv->getActualStock(1), 2);
}

TEST_F(ProductionLineTest, ProductionCompleteChangesOrderToConfirmed) {
    Order o(1, 1, "Lab-A", 1);
    o.setStatus(OrderStatus::PRODUCING);
    order_repo->save(o);

    EXPECT_CALL(mock_clock, now())
        .WillOnce(Return(t0))
        .WillOnce(Return(advanceSec(t0, 1.5)));

    prod->enqueue(1, 1, 1, 1.0);
    prod->tick();

    EXPECT_EQ(order_repo->findById(1)->getStatus(), OrderStatus::CONFIRMED);
    EXPECT_FALSE(prod->isProducing());
}

TEST_F(ProductionLineTest, NextJobStartsAfterCurrentCompletes) {
    Order o1(1, 1, "Lab-A", 1); o1.setStatus(OrderStatus::PRODUCING); order_repo->save(o1);
    Order o2(2, 1, "Lab-B", 1); o2.setStatus(OrderStatus::PRODUCING); order_repo->save(o2);

    EXPECT_CALL(mock_clock, now())
        .WillOnce(Return(t0))
        .WillOnce(Return(t0))
        .WillOnce(Return(advanceSec(t0, 1.5)))
        .WillOnce(Return(advanceSec(t0, 1.5)))
        .WillOnce(Return(advanceSec(t0, 3.0)));

    prod->enqueue(1, 1, 1, 1.0);
    prod->enqueue(2, 1, 1, 1.0);
    prod->tick();
    prod->tick();

    EXPECT_FALSE(prod->isProducing());
    EXPECT_EQ(order_repo->findById(1)->getStatus(), OrderStatus::CONFIRMED);
    EXPECT_EQ(order_repo->findById(2)->getStatus(), OrderStatus::CONFIRMED);
}
