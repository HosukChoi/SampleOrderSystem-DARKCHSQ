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

TEST_F(ProductionLineTest, QueuePersistsAcrossRestart) {
    const std::string queue_file = "test_pl_queue_tmp.json";
    EXPECT_CALL(mock_clock, now()).WillRepeatedly(Return(t0));
    {
        ProductionLine pl(*inv, *order_repo, mock_clock, queue_file);
        pl.enqueue(1, 1, 3, 1.0);
        pl.enqueue(2, 1, 2, 1.0);
        EXPECT_TRUE(pl.isProducing());
        EXPECT_EQ(pl.getQueueSize(), 1);
    }
    ProductionLine pl2(*inv, *order_repo, mock_clock, queue_file);
    EXPECT_TRUE(pl2.isProducing());
    EXPECT_EQ(pl2.getCurrentJob()->getOrderId(), 1);
    EXPECT_EQ(pl2.getQueueSize(), 1);
    fs::remove(queue_file);
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

TEST_F(ProductionLineTest, TickOnEmptyQueue_NoOp) {
    EXPECT_FALSE(prod->isProducing());
    prod->tick();
    EXPECT_FALSE(prod->isProducing());
    EXPECT_EQ(inv->getActualStock(1), 0);
}

TEST_F(ProductionLineTest, TickBeforeTimeElapsed_NoInventoryUpdate) {
    EXPECT_CALL(mock_clock, now())
        .WillOnce(Return(t0))
        .WillOnce(Return(advanceSec(t0, 0.5)));  // < avg_production_time=1.0

    prod->enqueue(1, 1, 2, 1.0);
    prod->tick();
    EXPECT_EQ(inv->getActualStock(1), 0);
    EXPECT_TRUE(prod->isProducing());
}

TEST_F(ProductionLineTest, MultipleJobsQueueOrder_FIFO) {
    EXPECT_CALL(mock_clock, now()).WillRepeatedly(Return(t0));
    prod->enqueue(10, 1, 2, 1.0);
    prod->enqueue(20, 1, 3, 1.0);
    prod->enqueue(30, 1, 1, 1.0);

    EXPECT_EQ(prod->getCurrentJob()->getOrderId(), 10);
    EXPECT_EQ(prod->getQueueSize(), 2);
    auto q = prod->getWaitingQueue();
    EXPECT_EQ(q.front().getOrderId(), 20); q.pop();
    EXPECT_EQ(q.front().getOrderId(), 30);
}

TEST_F(ProductionLineTest, ShortfallPersistedInQueue) {
    const std::string queue_file = "test_pl_shortfall_tmp.json";
    EXPECT_CALL(mock_clock, now()).WillRepeatedly(Return(t0));
    {
        ProductionLine pl(*inv, *order_repo, mock_clock, queue_file);
        pl.enqueue(1, 1, 5, 1.0, 3);  // shortfall=3, actual_qty=5
    }
    ProductionLine pl2(*inv, *order_repo, mock_clock, queue_file);
    ASSERT_NE(pl2.getCurrentJob(), nullptr);
    EXPECT_EQ(pl2.getCurrentJob()->getShortfall(), 3);
    EXPECT_EQ(pl2.getCurrentJob()->getActualQty(), 5);
    fs::remove(queue_file);
}
