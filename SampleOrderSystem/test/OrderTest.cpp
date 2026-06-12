// test/OrderTest.cpp
#include <gtest/gtest.h>
#include "domain/Order.h"

TEST(OrderTest, InitialStatusIsReserved) {
    Order o(1, 10, "Lab-A", 5);
    EXPECT_EQ(o.getStatus(), OrderStatus::RESERVED);
}

TEST(OrderTest, SetStatusChangesStatus) {
    Order o(1, 10, "Lab-A", 5);
    o.setStatus(OrderStatus::CONFIRMED);
    EXPECT_EQ(o.getStatus(), OrderStatus::CONFIRMED);
}

TEST(OrderTest, StoresFieldsCorrectly) {
    Order o(2, 10, "Lab-B", 3);
    EXPECT_EQ(o.getId(), 2);
    EXPECT_EQ(o.getSampleId(), 10);
    EXPECT_EQ(o.getCustomerName(), "Lab-B");
    EXPECT_EQ(o.getQuantity(), 3);
}

TEST(OrderTest, SetStatusToAllValidStatuses) {
    Order o(1, 1, "Lab-A", 1);
    o.setStatus(OrderStatus::PRODUCING);
    EXPECT_EQ(o.getStatus(), OrderStatus::PRODUCING);
    o.setStatus(OrderStatus::CONFIRMED);
    EXPECT_EQ(o.getStatus(), OrderStatus::CONFIRMED);
    o.setStatus(OrderStatus::RELEASE);
    EXPECT_EQ(o.getStatus(), OrderStatus::RELEASE);
    o.setStatus(OrderStatus::REJECTED);
    EXPECT_EQ(o.getStatus(), OrderStatus::REJECTED);
}

TEST(OrderTest, ZeroQuantityOrder) {
    Order o(1, 1, "Lab-A", 0);
    EXPECT_EQ(o.getQuantity(), 0);
    EXPECT_EQ(o.getStatus(), OrderStatus::RESERVED);
}
