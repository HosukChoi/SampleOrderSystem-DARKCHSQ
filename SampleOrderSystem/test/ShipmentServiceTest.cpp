// test/ShipmentServiceTest.cpp
#include <gtest/gtest.h>
#include "service/ShipmentService.h"
#include "repository/JsonOrderRepository.h"
#include "service/InventoryService.h"
#include <filesystem>

namespace fs = std::filesystem;

class ShipmentServiceTest : public ::testing::Test {
protected:
    const std::string order_file = "test_ss_orders_tmp.json";
    const std::string inv_file   = "test_ss_inv_tmp.json";
    JsonOrderRepository* order_repo;
    InventoryService*    inv;
    ShipmentService*     svc;

    void SetUp() override {
        order_repo = new JsonOrderRepository(order_file);
        inv        = new InventoryService(*order_repo, inv_file);
        svc        = new ShipmentService(*order_repo, *inv);
        inv->addActualStock(1, 100);
    }
    void TearDown() override {
        delete svc;
        delete inv;
        delete order_repo;
        fs::remove(order_file);
        fs::remove(inv_file);
    }
};

TEST_F(ShipmentServiceTest, ReleaseConfirmedOrderSucceeds) {
    Order o(1, 1, "Lab-A", 10); o.setStatus(OrderStatus::CONFIRMED);
    order_repo->save(o);
    EXPECT_TRUE(svc->release(1));
    EXPECT_EQ(order_repo->findById(1)->getStatus(), OrderStatus::RELEASE);
}

TEST_F(ShipmentServiceTest, ReleaseSubtractsActualStock) {
    Order o(1, 1, "Lab-A", 10); o.setStatus(OrderStatus::CONFIRMED);
    order_repo->save(o);
    svc->release(1);
    EXPECT_EQ(inv->getActualStock(1), 90);
}

TEST_F(ShipmentServiceTest, ReleaseNonConfirmedFails) {
    Order o(1, 1, "Lab-A", 10); o.setStatus(OrderStatus::PRODUCING);
    order_repo->save(o);
    EXPECT_FALSE(svc->release(1));
}

TEST_F(ShipmentServiceTest, GetConfirmedOrdersReturnsOnlyConfirmed) {
    Order o1(1, 1, "Lab-A", 5); o1.setStatus(OrderStatus::CONFIRMED); order_repo->save(o1);
    Order o2(2, 1, "Lab-B", 3); o2.setStatus(OrderStatus::PRODUCING); order_repo->save(o2);
    auto confirmed = svc->getConfirmedOrders();
    ASSERT_EQ(confirmed.size(), 1u);
    EXPECT_EQ(confirmed[0]->getId(), 1);
}

TEST_F(ShipmentServiceTest, ReleaseNonExistentOrderFails) {
    EXPECT_FALSE(svc->release(999));
}

TEST_F(ShipmentServiceTest, ReleaseAlreadyReleasedOrderFails) {
    Order o(1, 1, "Lab-A", 10); o.setStatus(OrderStatus::CONFIRMED);
    order_repo->save(o);
    EXPECT_TRUE(svc->release(1));
    EXPECT_FALSE(svc->release(1));
}

TEST_F(ShipmentServiceTest, ReleaseReservedOrderFails) {
    Order o(1, 1, "Lab-A", 10);  // RESERVED
    order_repo->save(o);
    EXPECT_FALSE(svc->release(1));
}

TEST_F(ShipmentServiceTest, GetConfirmedOrders_WhenEmpty) {
    EXPECT_TRUE(svc->getConfirmedOrders().empty());
}

TEST_F(ShipmentServiceTest, MultipleRelease_StockDecrementsAccumulate) {
    Order o1(1, 1, "Lab-A", 10); o1.setStatus(OrderStatus::CONFIRMED); order_repo->save(o1);
    Order o2(2, 1, "Lab-B", 25); o2.setStatus(OrderStatus::CONFIRMED); order_repo->save(o2);
    svc->release(1);
    svc->release(2);
    EXPECT_EQ(inv->getActualStock(1), 65);  // 100 - 10 - 25
}
