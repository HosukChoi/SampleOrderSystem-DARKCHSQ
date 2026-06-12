// test/OrderServiceTest.cpp
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "service/OrderService.h"
#include "repository/JsonSampleRepository.h"
#include "repository/JsonOrderRepository.h"
#include "service/InventoryService.h"
#include "service/ProductionLine.h"
#include <filesystem>

namespace fs = std::filesystem;
using ::testing::Return;

struct MockClock : public IClockProvider {
    MOCK_METHOD(std::chrono::system_clock::time_point, now, (), (const, override));
};

class OrderServiceTest : public ::testing::Test {
protected:
    const std::string sample_file = "test_os_samples_tmp.json";
    const std::string order_file  = "test_os_orders_tmp.json";
    const std::string inv_file    = "test_os_inv_tmp.json";

    JsonSampleRepository* sample_repo;
    JsonOrderRepository*  order_repo;
    InventoryService*     inv;
    ProductionLine*       prod_line;
    OrderService*         svc;
    MockClock             mock_clock;

    void SetUp() override {
        EXPECT_CALL(mock_clock, now())
            .WillRepeatedly(Return(std::chrono::system_clock::now()));
        sample_repo = new JsonSampleRepository(sample_file);
        order_repo  = new JsonOrderRepository(order_file);
        inv         = new InventoryService(*order_repo, inv_file);
        prod_line   = new ProductionLine(*inv, *order_repo, mock_clock);
        svc         = new OrderService(*order_repo, *sample_repo, *inv, *prod_line);
        sample_repo->save(Sample(1, "AlphaSi", 3.0, 0.9));
    }
    void TearDown() override {
        delete svc;
        delete prod_line;
        delete inv;
        delete order_repo;
        delete sample_repo;
        fs::remove(sample_file);
        fs::remove(order_file);
        fs::remove(inv_file);
    }
};

TEST_F(OrderServiceTest, PlaceOrderCreatesReservedOrder) {
    auto* o = svc->placeOrder(1, "Lab-A", 5);
    ASSERT_NE(o, nullptr);
    EXPECT_EQ(o->getStatus(), OrderStatus::RESERVED);
    EXPECT_EQ(o->getQuantity(), 5);
}

TEST_F(OrderServiceTest, PlaceOrderWithInvalidSampleReturnsNull) {
    EXPECT_EQ(svc->placeOrder(99, "Lab-A", 5), nullptr);
}

TEST_F(OrderServiceTest, ApproveOrderConfirmedWhenStockSufficient) {
    inv->addActualStock(1, 100);
    auto* o = svc->placeOrder(1, "Lab-A", 5);
    EXPECT_TRUE(svc->approveOrder(o->getId()));
    EXPECT_EQ(o->getStatus(), OrderStatus::CONFIRMED);
}

TEST_F(OrderServiceTest, ApproveOrderProducingWhenStockInsufficient) {
    auto* o = svc->placeOrder(1, "Lab-A", 5);  // 재고 0
    EXPECT_TRUE(svc->approveOrder(o->getId()));
    EXPECT_EQ(o->getStatus(), OrderStatus::PRODUCING);
}

TEST_F(OrderServiceTest, RejectOrderSetsRejectedStatus) {
    auto* o = svc->placeOrder(1, "Lab-A", 5);
    EXPECT_TRUE(svc->rejectOrder(o->getId()));
    EXPECT_EQ(o->getStatus(), OrderStatus::REJECTED);
}

TEST_F(OrderServiceTest, ApproveNonReservedOrderFails) {
    auto* o = svc->placeOrder(1, "Lab-A", 5);
    svc->approveOrder(o->getId());           // PRODUCING
    EXPECT_FALSE(svc->approveOrder(o->getId())); // 재승인 불가
}

TEST_F(OrderServiceTest, AvailableStockConsideredOnApproval) {
    // 재고 10, 기존 CONFIRMED 주문 8 → 가용 2 < 신규 5 → PRODUCING
    inv->addActualStock(1, 10);
    Order existing(order_repo->nextId(), 1, "Lab-X", 8);
    existing.setStatus(OrderStatus::CONFIRMED);
    order_repo->save(existing);

    auto* o = svc->placeOrder(1, "Lab-A", 5);
    svc->approveOrder(o->getId());
    EXPECT_EQ(o->getStatus(), OrderStatus::PRODUCING);
}

TEST_F(OrderServiceTest, ApproveOrder_ExactStockMatch) {
    // available == quantity → CONFIRMED
    inv->addActualStock(1, 5);
    auto* o = svc->placeOrder(1, "Lab-A", 5);
    EXPECT_TRUE(svc->approveOrder(o->getId()));
    EXPECT_EQ(o->getStatus(), OrderStatus::CONFIRMED);
}

TEST_F(OrderServiceTest, ApproveOrder_ShortfallCalculation) {
    // AlphaSi: yield=0.9, actual=3, order=5
    // available=3, shortfall=5-3=2, actual_qty=ceil(2/(0.9*0.9))=ceil(2.469)=3
    inv->addActualStock(1, 3);
    auto* o = svc->placeOrder(1, "Lab-A", 5);
    EXPECT_TRUE(svc->approveOrder(o->getId()));
    EXPECT_EQ(o->getStatus(), OrderStatus::PRODUCING);
    ASSERT_NE(prod_line->getCurrentJob(), nullptr);
    EXPECT_EQ(prod_line->getCurrentJob()->getActualQty(), 3);
}

TEST_F(OrderServiceTest, RejectNonReservedOrderFails) {
    auto* o = svc->placeOrder(1, "Lab-A", 5);
    svc->approveOrder(o->getId());  // now PRODUCING
    EXPECT_FALSE(svc->rejectOrder(o->getId()));
}

TEST_F(OrderServiceTest, ApproveNonExistentOrderFails) {
    EXPECT_FALSE(svc->approveOrder(999));
}

TEST_F(OrderServiceTest, RejectNonExistentOrderFails) {
    EXPECT_FALSE(svc->rejectOrder(999));
}

TEST_F(OrderServiceTest, GetReservedOrders_ReturnsOnlyReserved) {
    auto* o1 = svc->placeOrder(1, "Lab-A", 3);
    auto* o2 = svc->placeOrder(1, "Lab-B", 2);
    svc->approveOrder(o1->getId());  // becomes PRODUCING (no stock)

    auto reserved = svc->getReservedOrders();
    ASSERT_EQ(reserved.size(), 1u);
    EXPECT_EQ(reserved[0]->getId(), o2->getId());
}

TEST_F(OrderServiceTest, GetAllOrders_IncludesAllStatuses) {
    auto* o1 = svc->placeOrder(1, "Lab-A", 3);
    auto* o2 = svc->placeOrder(1, "Lab-B", 2);
    svc->rejectOrder(o2->getId());
    EXPECT_EQ(svc->getAllOrders().size(), 2u);
}
