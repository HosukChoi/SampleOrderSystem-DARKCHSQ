# 반도체 시료 생산주문관리 시스템 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use `superpowers:subagent-driven-development` (recommended) or `superpowers:executing-plans` to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** S-Semi의 반도체 시료 생산·주문·재고·출고를 통합 관리하는 C++ 콘솔 애플리케이션을 구현한다.

**Architecture:** 도메인 모델 → 인터페이스 → 리포지토리 → 서비스 → View 레이어 순으로 Bottom-up 방식으로 구현한다. 생산 라인은 `std::chrono` 기반 시간 시뮬레이션(tick 폴링)으로 단일 스레드에서 실시간 재고 업데이트를 구현한다. 데이터는 인메모리로 관리하며, 각 Repository가 생성자에서 JSON 파일을 로드하고 모든 write 시 자동 저장한다.

**Tech Stack:** C++20, Visual Studio 2022, gmock 1.11.0 (NuGet), `nlohmann/json` (NuGet), `<chrono>`, `<queue>`, `<cmath>`

**TDD 규칙:** 모든 구현 Task는 아래 순서를 반드시 따른다.
1. 테스트 작성 (컴파일 실패 상태)
2. 빌드 → 실패 확인
3. 구현 작성
4. 빌드 & 실행 → 테스트 통과 확인
5. 커밋

---

## 파일 구조

```
SampleOrderSystem/
├── SampleOrderSystem.vcxproj     # Visual Studio 프로젝트 (기존)
├── main.cpp                      # RUN_ALL_TESTS() 진입점 (기존)
├── domain/
│   ├── OrderStatus.h
│   ├── Sample.h / .cpp
│   ├── Order.h / .cpp
│   └── ProductionJob.h / .cpp
├── interface/
│   ├── IComponent.h
│   ├── IClockProvider.h          # 시간 추상화 인터페이스
│   ├── ISampleRepository.h
│   └── IOrderRepository.h
├── repository/
│   ├── JsonSampleRepository.h / .cpp   # 생성자에 filePath, 내부 load/save
│   └── JsonOrderRepository.h / .cpp   # 동일 패턴
├── service/
│   ├── RealClock.h               # IClockProvider 실제 구현 (헤더 전용)
│   ├── InventoryService.h / .cpp # filePath를 받아 JSON 자동 저장/로드
│   ├── SampleService.h / .cpp
│   ├── OrderService.h / .cpp
│   ├── ProductionLine.h / .cpp
│   └── ShipmentService.h / .cpp
├── controller/
│   └── AppController.h / .cpp   # 모든 services + MainView 소유, run() 메인 루프
├── data/                         # 런타임 생성. JSON 저장 위치
│   ├── samples.json
│   ├── orders.json
│   └── inventory.json
├── view/
│   ├── ConsoleUtils.h / .cpp
│   ├── MainView.h / .cpp
│   ├── SampleView.h / .cpp
│   ├── OrderView.h / .cpp
│   ├── MonitoringView.h / .cpp
│   ├── ProductionView.h / .cpp
│   └── ShipmentView.h / .cpp
└── test/
    ├── SampleTest.cpp
    ├── OrderTest.cpp
    ├── InventoryServiceTest.cpp
    ├── SampleServiceTest.cpp
    ├── OrderServiceTest.cpp
    ├── ProductionLineTest.cpp
    └── ShipmentServiceTest.cpp
```

> vcxproj에 새 파일 추가 시: 프로젝트 우클릭 → 추가 → 기존 항목 또는 새 항목

---

## Task 1: 도메인 — OrderStatus, Sample

**Files:**
- Create: `SampleOrderSystem/domain/OrderStatus.h`
- Create: `SampleOrderSystem/domain/Sample.h`
- Create: `SampleOrderSystem/domain/Sample.cpp`
- Create: `SampleOrderSystem/test/SampleTest.cpp`

### Step 1-1: 테스트 먼저 작성

- [ ] `test/SampleTest.cpp` 생성 후 vcxproj에 추가

```cpp
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
```

- [ ] 빌드 → 컴파일 오류 확인 (Sample 미정의)

### Step 1-2: OrderStatus 구현

- [ ] `domain/OrderStatus.h` 생성 후 vcxproj에 추가

```cpp
// domain/OrderStatus.h
#pragma once

enum class OrderStatus {
    RESERVED,
    REJECTED,
    PRODUCING,
    CONFIRMED,
    RELEASE
};

inline const char* toString(OrderStatus s) {
    switch (s) {
        case OrderStatus::RESERVED:  return "RESERVED";
        case OrderStatus::REJECTED:  return "REJECTED";
        case OrderStatus::PRODUCING: return "PRODUCING";
        case OrderStatus::CONFIRMED: return "CONFIRMED";
        case OrderStatus::RELEASE:   return "RELEASE";
    }
    return "UNKNOWN";
}
```

### Step 1-3: Sample 구현

- [ ] `domain/Sample.h` 생성 후 vcxproj에 추가

```cpp
// domain/Sample.h
#pragma once
#include <string>

class Sample {
public:
    Sample(int id, const std::string& name, double avg_production_time, double yield);

    int getId() const;
    const std::string& getName() const;
    double getAvgProductionTime() const;  // 단위: 초
    double getYield() const;              // 0.0 ~ 1.0

private:
    int id_;
    std::string name_;
    double avg_production_time_;
    double yield_;
};
```

- [ ] `domain/Sample.cpp` 생성 후 vcxproj에 추가

```cpp
// domain/Sample.cpp
#include "domain/Sample.h"

Sample::Sample(int id, const std::string& name, double avg_production_time, double yield)
    : id_(id), name_(name), avg_production_time_(avg_production_time), yield_(yield) {}

int Sample::getId() const { return id_; }
const std::string& Sample::getName() const { return name_; }
double Sample::getAvgProductionTime() const { return avg_production_time_; }
double Sample::getYield() const { return yield_; }
```

- [ ] 빌드 & 실행 → `SampleTest.StoresFieldsCorrectly` PASSED 확인

- [ ] 커밋

```
[feat](domain): OrderStatus enum 및 Sample 클래스 추가
```

---

## Task 2: 도메인 — Order, ProductionJob

**Files:**
- Create: `SampleOrderSystem/domain/Order.h / .cpp`
- Create: `SampleOrderSystem/domain/ProductionJob.h / .cpp`
- Create: `SampleOrderSystem/test/OrderTest.cpp`

### Step 2-1: 테스트 먼저 작성

- [ ] `test/OrderTest.cpp` 생성 후 vcxproj에 추가

```cpp
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
```

- [ ] 빌드 → 컴파일 오류 확인 (Order 미정의)

### Step 2-2: Order 구현

- [ ] `domain/Order.h` 생성 후 vcxproj에 추가

```cpp
// domain/Order.h
#pragma once
#include <string>
#include "domain/OrderStatus.h"

class Order {
public:
    Order(int id, int sample_id, const std::string& customer_name, int quantity);

    int getId() const;
    int getSampleId() const;
    const std::string& getCustomerName() const;
    int getQuantity() const;
    OrderStatus getStatus() const;
    void setStatus(OrderStatus status);

private:
    int id_;
    int sample_id_;
    std::string customer_name_;
    int quantity_;
    OrderStatus status_;
};
```

- [ ] `domain/Order.cpp` 생성 후 vcxproj에 추가

```cpp
// domain/Order.cpp
#include "domain/Order.h"

Order::Order(int id, int sample_id, const std::string& customer_name, int quantity)
    : id_(id), sample_id_(sample_id), customer_name_(customer_name),
      quantity_(quantity), status_(OrderStatus::RESERVED) {}

int Order::getId() const { return id_; }
int Order::getSampleId() const { return sample_id_; }
const std::string& Order::getCustomerName() const { return customer_name_; }
int Order::getQuantity() const { return quantity_; }
OrderStatus Order::getStatus() const { return status_; }
void Order::setStatus(OrderStatus status) { status_ = status; }
```

- [ ] 빌드 & 실행 → `OrderTest.*` 3개 PASSED 확인

### Step 2-3: ProductionJob 구현 (테스트는 ProductionLine Task에서 통합 검증)

- [ ] `domain/ProductionJob.h` 생성 후 vcxproj에 추가

```cpp
// domain/ProductionJob.h
#pragma once
#include <chrono>
#include "interface/IClockProvider.h"

class ProductionJob {
public:
    // clock: 실제 동작 시 RealClock, 테스트 시 MockClock 주입
    ProductionJob(int order_id, int sample_id, int actual_qty,
                  double avg_production_time, IClockProvider& clock);

    int getOrderId() const;
    int getSampleId() const;
    int getActualQty() const;
    int getProducedQty() const;
    void addProduced(int qty);
    bool isComplete() const;
    int calcNewlyProduced();    // clock.now()를 내부에서 호출
    void resetLastTick();       // enqueue 시점에 last_tick 초기화

private:
    int order_id_;
    int sample_id_;
    int actual_qty_;
    int produced_qty_;
    double avg_production_time_;
    IClockProvider& clock_;
    std::chrono::system_clock::time_point last_tick_;
    double accumulated_time_;
};
```

- [ ] `domain/ProductionJob.cpp` 생성 후 vcxproj에 추가

```cpp
// domain/ProductionJob.cpp
#include "domain/ProductionJob.h"
#include <algorithm>
#include <cmath>

ProductionJob::ProductionJob(int order_id, int sample_id, int actual_qty,
                              double avg_production_time, IClockProvider& clock)
    : order_id_(order_id), sample_id_(sample_id), actual_qty_(actual_qty),
      produced_qty_(0), avg_production_time_(avg_production_time),
      clock_(clock), last_tick_(clock.now()), accumulated_time_(0.0) {}

int ProductionJob::getOrderId() const { return order_id_; }
int ProductionJob::getSampleId() const { return sample_id_; }
int ProductionJob::getActualQty() const { return actual_qty_; }
int ProductionJob::getProducedQty() const { return produced_qty_; }
void ProductionJob::addProduced(int qty) { produced_qty_ += qty; }
bool ProductionJob::isComplete() const { return produced_qty_ >= actual_qty_; }

int ProductionJob::calcNewlyProduced() {
    auto now = clock_.now();
    double elapsed = std::chrono::duration<double>(now - last_tick_).count();
    last_tick_ = now;
    accumulated_time_ += elapsed;
    int newly = static_cast<int>(accumulated_time_ / avg_production_time_);
    accumulated_time_ -= newly * avg_production_time_;
    return std::min(newly, actual_qty_ - produced_qty_);
}

void ProductionJob::resetLastTick() {
    last_tick_ = clock_.now();
    accumulated_time_ = 0.0;
}
```

- [ ] 빌드 확인 (기존 테스트 포함 전체 PASSED)

- [ ] 커밋

```
[feat](domain): Order 및 ProductionJob 클래스 추가
```

---

## Task 3: 인터페이스 정의

**Files:**
- Create: `SampleOrderSystem/interface/IComponent.h`
- Create: `SampleOrderSystem/interface/ISampleRepository.h`
- Create: `SampleOrderSystem/interface/IOrderRepository.h`

> 인터페이스는 순수 추상 클래스로 테스트 대상이 아님. 리포지토리 Task에서 구현체를 통해 간접 검증.

- [ ] `interface/IComponent.h` 생성 후 vcxproj에 추가

```cpp
// interface/IComponent.h
#pragma once

#define interface struct

interface IComponent {
    virtual ~IComponent() = default;
    virtual int getId() const = 0;
};
```

- [ ] `interface/IClockProvider.h` 생성 후 vcxproj에 추가

```cpp
// interface/IClockProvider.h
#pragma once
#include <chrono>

#define interface struct

interface IClockProvider {
    virtual ~IClockProvider() = default;
    virtual std::chrono::system_clock::time_point now() const = 0;
};
```

- [ ] `service/RealClock.h` 생성 후 vcxproj에 추가 (헤더 전용, cpp 없음)

```cpp
// service/RealClock.h
#pragma once
#include "interface/IClockProvider.h"

class RealClock : public IClockProvider {
public:
    std::chrono::system_clock::time_point now() const override {
        return std::chrono::system_clock::now();
    }
};
```

- [ ] `interface/ISampleRepository.h` 생성 후 vcxproj에 추가

```cpp
// interface/ISampleRepository.h
#pragma once
#include <vector>
#include <string>
#include "interface/IComponent.h"
#include "domain/Sample.h"

interface ISampleRepository : public IComponent {
    virtual void save(const Sample& sample) = 0;
    virtual Sample* findById(int id) = 0;
    virtual std::vector<Sample*> findAll() = 0;
    virtual std::vector<Sample*> findByName(const std::string& keyword) = 0;
    virtual int getId() const override = 0;
};
```

- [ ] `interface/IOrderRepository.h` 생성 후 vcxproj에 추가

```cpp
// interface/IOrderRepository.h
#pragma once
#include <vector>
#include "interface/IComponent.h"
#include "domain/Order.h"
#include "domain/OrderStatus.h"

interface IOrderRepository : public IComponent {
    virtual void save(const Order& order) = 0;
    virtual Order* findById(int id) = 0;
    virtual std::vector<Order*> findAll() = 0;
    virtual std::vector<Order*> findByStatus(OrderStatus status) = 0;
    virtual int nextId() = 0;
    virtual int getId() const override = 0;
};
```

- [ ] 빌드 확인 (기존 테스트 전체 PASSED)

- [ ] 커밋

```
[feat](interface): ISampleRepository, IOrderRepository 인터페이스 정의
```

---

## Task 4: 리포지토리 구현 (JSON 영속성)

> DataPersistence POC 반영: 별도 PersistenceManager 없음. 각 Repository가 생성자에서 파일을 load하고, 모든 write 메서드에서 자동으로 save한다.

**Install:** `nlohmann.json` NuGet 패키지 (프로젝트 우클릭 → NuGet 패키지 관리 → `nlohmann.json` 검색 후 설치)

**Files:**
- Create: `SampleOrderSystem/repository/JsonSampleRepository.h / .cpp`
- Create: `SampleOrderSystem/repository/JsonOrderRepository.h / .cpp`
- Create: `SampleOrderSystem/test/JsonSampleRepositoryTest.cpp`
- Create: `SampleOrderSystem/test/JsonOrderRepositoryTest.cpp`

**저장 파일 및 구조:**

```
data/samples.json   → [{"id":1,"name":"AlphaSi","avg_production_time":3.0,"yield":0.9}, ...]
data/orders.json    → [{"id":1,"sample_id":1,"customer_name":"Lab-A","quantity":5,"status":"CONFIRMED"}, ...]
```

- [ ] `repository/JsonSampleRepository.h` 생성 후 vcxproj에 추가

```cpp
// repository/JsonSampleRepository.h
#pragma once
#include <map>
#include <string>
#include "interface/ISampleRepository.h"

class JsonSampleRepository : public ISampleRepository {
public:
    explicit JsonSampleRepository(const std::string& filePath);

    int getId() const override { return 1; }
    void save(const Sample& sample) override;
    Sample* findById(int id) override;
    std::vector<Sample*> findAll() override;
    std::vector<Sample*> findByName(const std::string& keyword) override;

private:
    std::string       file_path_;
    std::map<int, Sample> store_;

    void load();
    void persist() const;
};
```

- [ ] `repository/JsonSampleRepository.cpp` 생성 후 vcxproj에 추가

```cpp
// repository/JsonSampleRepository.cpp
#include "repository/JsonSampleRepository.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>

using json = nlohmann::json;
namespace fs = std::filesystem;

JsonSampleRepository::JsonSampleRepository(const std::string& filePath)
    : file_path_(filePath) {
    load();
}

void JsonSampleRepository::load() {
    std::ifstream f(file_path_);
    if (!f.is_open()) return;
    json j; f >> j;
    for (auto& item : j)
        store_.emplace(item["id"].get<int>(),
            Sample(item["id"].get<int>(),
                   item["name"].get<std::string>(),
                   item["avg_production_time"].get<double>(),
                   item["yield"].get<double>()));
}

void JsonSampleRepository::persist() const {
    auto parent = fs::path(file_path_).parent_path();
    if (!parent.empty()) fs::create_directories(parent);
    json j = json::array();
    for (auto& [id, s] : store_)
        j.push_back({{"id", s.getId()},
                     {"name", s.getName()},
                     {"avg_production_time", s.getAvgProductionTime()},
                     {"yield", s.getYield()}});
    std::ofstream(file_path_) << j.dump(2);
}

void JsonSampleRepository::save(const Sample& sample) {
    store_.emplace(sample.getId(), sample);
    persist();
}

Sample* JsonSampleRepository::findById(int id) {
    auto it = store_.find(id);
    return it != store_.end() ? &it->second : nullptr;
}

std::vector<Sample*> JsonSampleRepository::findAll() {
    std::vector<Sample*> result;
    for (auto& [id, sample] : store_)
        result.push_back(&sample);
    return result;
}

std::vector<Sample*> JsonSampleRepository::findByName(const std::string& keyword) {
    std::vector<Sample*> result;
    for (auto& [id, sample] : store_)
        if (sample.getName().find(keyword) != std::string::npos)
            result.push_back(&sample);
    return result;
}
```

- [ ] `repository/JsonOrderRepository.h` 생성 후 vcxproj에 추가

```cpp
// repository/JsonOrderRepository.h
#pragma once
#include <map>
#include <string>
#include "interface/IOrderRepository.h"

class JsonOrderRepository : public IOrderRepository {
public:
    explicit JsonOrderRepository(const std::string& filePath);

    int getId() const override { return 2; }
    void save(const Order& order) override;
    Order* findById(int id) override;
    std::vector<Order*> findAll() override;
    std::vector<Order*> findByStatus(OrderStatus status) override;
    int nextId() override;

private:
    std::string       file_path_;
    std::map<int, Order> store_;
    int               next_id_ = 1;

    void load();
    void persist() const;
};
```

- [ ] `repository/JsonOrderRepository.cpp` 생성 후 vcxproj에 추가

```cpp
// repository/JsonOrderRepository.cpp
#include "repository/JsonOrderRepository.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>

using json = nlohmann::json;
namespace fs = std::filesystem;

static OrderStatus strToStatus(const std::string& s) {
    if (s == "RESERVED")  return OrderStatus::RESERVED;
    if (s == "REJECTED")  return OrderStatus::REJECTED;
    if (s == "PRODUCING") return OrderStatus::PRODUCING;
    if (s == "CONFIRMED") return OrderStatus::CONFIRMED;
    return OrderStatus::RELEASE;
}

JsonOrderRepository::JsonOrderRepository(const std::string& filePath)
    : file_path_(filePath) {
    load();
}

void JsonOrderRepository::load() {
    std::ifstream f(file_path_);
    if (!f.is_open()) return;
    json j; f >> j;
    for (auto& item : j) {
        Order o(item["id"].get<int>(),
                item["sample_id"].get<int>(),
                item["customer_name"].get<std::string>(),
                item["quantity"].get<int>());
        o.setStatus(strToStatus(item["status"].get<std::string>()));
        int id = o.getId();
        store_.emplace(id, o);
        if (id >= next_id_) next_id_ = id + 1;
    }
}

void JsonOrderRepository::persist() const {
    auto parent = fs::path(file_path_).parent_path();
    if (!parent.empty()) fs::create_directories(parent);
    json j = json::array();
    for (auto& [id, o] : store_)
        j.push_back({{"id",            o.getId()},
                     {"sample_id",     o.getSampleId()},
                     {"customer_name", o.getCustomerName()},
                     {"quantity",      o.getQuantity()},
                     {"status",        toString(o.getStatus())}});
    std::ofstream(file_path_) << j.dump(2);
}

void JsonOrderRepository::save(const Order& order) {
    store_.emplace(order.getId(), order);
    persist();
}

Order* JsonOrderRepository::findById(int id) {
    auto it = store_.find(id);
    return it != store_.end() ? &it->second : nullptr;
}

std::vector<Order*> JsonOrderRepository::findAll() {
    std::vector<Order*> result;
    for (auto& [id, order] : store_)
        result.push_back(&order);
    return result;
}

std::vector<Order*> JsonOrderRepository::findByStatus(OrderStatus status) {
    std::vector<Order*> result;
    for (auto& [id, order] : store_)
        if (order.getStatus() == status)
            result.push_back(&order);
    return result;
}

int JsonOrderRepository::nextId() { return next_id_++; }
```

- [ ] `test/JsonSampleRepositoryTest.cpp` 생성 후 vcxproj에 추가

```cpp
// test/JsonSampleRepositoryTest.cpp
#include <gtest/gtest.h>
#include "repository/JsonSampleRepository.h"
#include <filesystem>

namespace fs = std::filesystem;

class JsonSampleRepositoryTest : public ::testing::Test {
protected:
    const std::string file = "test_samples_tmp.json";
    JsonSampleRepository* repo;

    void SetUp() override { repo = new JsonSampleRepository(file); }
    void TearDown() override { delete repo; fs::remove(file); }
};

TEST_F(JsonSampleRepositoryTest, FindByIdReturnsNullWhenEmpty) {
    EXPECT_EQ(repo->findById(1), nullptr);
}
TEST_F(JsonSampleRepositoryTest, SaveAndFindById) {
    repo->save(Sample(1, "AlphaSi", 3.0, 0.9));
    Sample* s = repo->findById(1);
    ASSERT_NE(s, nullptr);
    EXPECT_EQ(s->getName(), "AlphaSi");
}
TEST_F(JsonSampleRepositoryTest, FindAllReturnsAllSaved) {
    repo->save(Sample(1, "AlphaSi", 3.0, 0.9));
    repo->save(Sample(2, "BetaSi",  5.0, 0.85));
    EXPECT_EQ(repo->findAll().size(), 2u);
}
TEST_F(JsonSampleRepositoryTest, FindByNameKeywordMatch) {
    repo->save(Sample(1, "AlphaSi", 3.0, 0.9));
    repo->save(Sample(2, "BetaSi",  5.0, 0.85));
    auto partial = repo->findByName("Alpha");
    ASSERT_EQ(partial.size(), 1u);
    EXPECT_EQ(partial[0]->getName(), "AlphaSi");
}
TEST_F(JsonSampleRepositoryTest, PersistAndReloadRoundTrip) {
    repo->save(Sample(1, "AlphaSi", 3.0, 0.9));
    delete repo;
    repo = new JsonSampleRepository(file);
    ASSERT_NE(repo->findById(1), nullptr);
    EXPECT_EQ(repo->findById(1)->getName(), "AlphaSi");
}
```

- [ ] `test/JsonOrderRepositoryTest.cpp` 생성 후 vcxproj에 추가

```cpp
// test/JsonOrderRepositoryTest.cpp
#include <gtest/gtest.h>
#include "repository/JsonOrderRepository.h"
#include <filesystem>

namespace fs = std::filesystem;

class JsonOrderRepositoryTest : public ::testing::Test {
protected:
    const std::string file = "test_orders_tmp.json";
    JsonOrderRepository* repo;

    void SetUp() override { repo = new JsonOrderRepository(file); }
    void TearDown() override { delete repo; fs::remove(file); }
};

TEST_F(JsonOrderRepositoryTest, FindByIdReturnsNullWhenEmpty) {
    EXPECT_EQ(repo->findById(1), nullptr);
}
TEST_F(JsonOrderRepositoryTest, SaveAndFindById) {
    repo->save(Order(1, 10, "Lab-A", 5));
    Order* o = repo->findById(1);
    ASSERT_NE(o, nullptr);
    EXPECT_EQ(o->getCustomerName(), "Lab-A");
    EXPECT_EQ(o->getStatus(), OrderStatus::RESERVED);
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
    EXPECT_EQ(repo->findByStatus(OrderStatus::RESERVED).size(),  1u);
}
TEST_F(JsonOrderRepositoryTest, NextIdIncrements) {
    EXPECT_EQ(repo->nextId(), 1);
    EXPECT_EQ(repo->nextId(), 2);
}
TEST_F(JsonOrderRepositoryTest, PersistAndReloadRoundTrip) {
    Order o(1, 10, "Lab-A", 5); o.setStatus(OrderStatus::CONFIRMED);
    repo->save(o);
    delete repo;
    repo = new JsonOrderRepository(file);
    Order* found = repo->findById(1);
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->getStatus(), OrderStatus::CONFIRMED);
}
```

- [ ] 빌드 & 실행 → `JsonSampleRepositoryTest.*` 5개, `JsonOrderRepositoryTest.*` 6개 PASSED 확인

- [ ] 커밋

```
[feat](repository): JsonSampleRepository, JsonOrderRepository - JSON 자동 영속성 구현
[test](repository): JsonSampleRepository, JsonOrderRepository 단위 테스트 추가
```

---

## Task 5: InventoryService — 실재/필요/가용 재고 + JSON 영속성

> DataPersistence POC 반영: InventoryService도 filePath를 받아 생성자에서 load, 모든 write 시 자동 save.

**Files:**
- Create: `SampleOrderSystem/service/InventoryService.h / .cpp`
- Create: `SampleOrderSystem/test/InventoryServiceTest.cpp`

### Step 5-1: 테스트 먼저 작성

- [ ] `test/InventoryServiceTest.cpp` 생성 후 vcxproj에 추가

```cpp
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
    // 새 인스턴스에서 동일 파일 로드
    JsonOrderRepository order2(order_file);
    InventoryService inv2(order2, inv_file);
    EXPECT_EQ(inv2.getActualStock(1), 77);
}
```

- [ ] 빌드 → 컴파일 오류 확인 (InventoryService 미정의)

### Step 5-2: InventoryService 구현

- [ ] `service/InventoryService.h` 생성 후 vcxproj에 추가

```cpp
// service/InventoryService.h
#pragma once
#include <map>
#include <string>
#include "interface/IOrderRepository.h"

class InventoryService {
public:
    // filePath: inventory.json 경로. 생성자에서 load, write 시 자동 save.
    explicit InventoryService(IOrderRepository& order_repo,
                              const std::string& filePath = "data/inventory.json");

    int getActualStock(int sample_id) const;
    void addActualStock(int sample_id, int qty);
    void subtractActualStock(int sample_id, int qty);
    int getRequiredStock(int sample_id) const;
    int getAvailableStock(int sample_id) const;
    const char* getStockStatus(int sample_id) const;

private:
    IOrderRepository& order_repo_;
    std::string       file_path_;
    std::map<int, int> actual_stock_;

    void load();
    void persist() const;
};
```

- [ ] `service/InventoryService.cpp` 생성 후 vcxproj에 추가

```cpp
// service/InventoryService.cpp
#include "service/InventoryService.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>

using json = nlohmann::json;
namespace fs = std::filesystem;

InventoryService::InventoryService(IOrderRepository& order_repo,
                                   const std::string& filePath)
    : order_repo_(order_repo), file_path_(filePath) {
    load();
}

void InventoryService::load() {
    std::ifstream f(file_path_);
    if (!f.is_open()) return;
    json j; f >> j;
    for (auto& [key, val] : j.items())
        actual_stock_[std::stoi(key)] = val.get<int>();
}

void InventoryService::persist() const {
    fs::create_directories(fs::path(file_path_).parent_path());
    json j;
    for (auto& [id, qty] : actual_stock_)
        j[std::to_string(id)] = qty;
    std::ofstream(file_path_) << j.dump(2);
}

int InventoryService::getActualStock(int sample_id) const {
    auto it = actual_stock_.find(sample_id);
    return it != actual_stock_.end() ? it->second : 0;
}

void InventoryService::addActualStock(int sample_id, int qty) {
    actual_stock_[sample_id] += qty;
    persist();
}

void InventoryService::subtractActualStock(int sample_id, int qty) {
    actual_stock_[sample_id] -= qty;
    persist();
}

int InventoryService::getRequiredStock(int sample_id) const {
    int total = 0;
    for (auto* order : order_repo_.findAll()) {
        if (order->getSampleId() == sample_id &&
            (order->getStatus() == OrderStatus::CONFIRMED ||
             order->getStatus() == OrderStatus::PRODUCING)) {
            total += order->getQuantity();
        }
    }
    return total;
}

int InventoryService::getAvailableStock(int sample_id) const {
    return getActualStock(sample_id) - getRequiredStock(sample_id);
}

const char* InventoryService::getStockStatus(int sample_id) const {
    if (getActualStock(sample_id) == 0) return "고갈";
    if (getAvailableStock(sample_id) <= 0) return "부족";
    return "여유";
}
```

- [ ] 빌드 & 실행 → `InventoryServiceTest.*` 9개 PASSED 확인

- [ ] 커밋

```
[feat](service): InventoryService - 실재/필요/가용 재고 계산 + JSON 자동 영속성 구현
```

---

## Task 6: SampleService — 시료 등록·조회·검색

**Files:**
- Create: `SampleOrderSystem/service/SampleService.h / .cpp`
- Create: `SampleOrderSystem/test/SampleServiceTest.cpp`

### Step 6-1: 테스트 먼저 작성

- [ ] `test/SampleServiceTest.cpp` 생성 후 vcxproj에 추가

```cpp
// test/SampleServiceTest.cpp
#include <gtest/gtest.h>
#include "service/SampleService.h"
#include "repository/JsonSampleRepository.h"
#include <filesystem>

namespace fs = std::filesystem;

class SampleServiceTest : public ::testing::Test {
protected:
    const std::string sample_file = "test_samples_tmp.json";
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
```

- [ ] 빌드 → 컴파일 오류 확인 (SampleService 미정의)

### Step 6-2: SampleService 구현

- [ ] `service/SampleService.h` 생성 후 vcxproj에 추가

```cpp
// service/SampleService.h
#pragma once
#include <vector>
#include <string>
#include "interface/ISampleRepository.h"

class SampleService {
public:
    explicit SampleService(ISampleRepository& sample_repo);

    bool registerSample(int id, const std::string& name,
                        double avg_production_time, double yield);
    std::vector<Sample*> getAllSamples();
    Sample* findById(int id);
    std::vector<Sample*> searchByName(const std::string& keyword);

private:
    ISampleRepository& sample_repo_;
};
```

- [ ] `service/SampleService.cpp` 생성 후 vcxproj에 추가

```cpp
// service/SampleService.cpp
#include "service/SampleService.h"

SampleService::SampleService(ISampleRepository& sample_repo)
    : sample_repo_(sample_repo) {}

bool SampleService::registerSample(int id, const std::string& name,
                                   double avg_production_time, double yield) {
    if (sample_repo_.findById(id) != nullptr) return false;
    sample_repo_.save(Sample(id, name, avg_production_time, yield));
    return true;
}

std::vector<Sample*> SampleService::getAllSamples() {
    return sample_repo_.findAll();
}

Sample* SampleService::findById(int id) {
    return sample_repo_.findById(id);
}

std::vector<Sample*> SampleService::searchByName(const std::string& keyword) {
    return sample_repo_.findByName(keyword);
}
```

- [ ] 빌드 & 실행 → `SampleServiceTest.*` 6개 PASSED 확인

- [ ] 커밋

```
[feat](service): SampleService - 시료 등록/조회/검색 구현
```

---

## Task 7: OrderService — 주문 접수·승인·거절

**Files:**
- Create: `SampleOrderSystem/service/OrderService.h / .cpp`
- Create: `SampleOrderSystem/test/OrderServiceTest.cpp`

### Step 7-1: Mock 정의 + 테스트 먼저 작성

> OrderService는 ISampleRepository, IOrderRepository에 의존 → gmock으로 Mock 주입

- [ ] `test/OrderServiceTest.cpp` 생성 후 vcxproj에 추가

```cpp
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
        // 기본 시료 등록
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
```

- [ ] 빌드 → 컴파일 오류 확인 (OrderService, ProductionLine 미정의)

### Step 7-2: ProductionLine stub 먼저 생성 (OrderService 컴파일용)

> ProductionLine 전체 구현은 Task 8에서. 여기서는 헤더만 생성해 컴파일을 통과시킨다.

- [ ] `service/ProductionLine.h` 생성 후 vcxproj에 추가 (stub)

```cpp
// service/ProductionLine.h
#pragma once
#include <queue>
#include <memory>
#include "domain/ProductionJob.h"
#include "service/InventoryService.h"
#include "interface/IOrderRepository.h"
#include "interface/IClockProvider.h"

class ProductionLine {
public:
    // clock: 실제 동작 시 RealClock, 테스트 시 MockClock 주입
    ProductionLine(InventoryService& inventory, IOrderRepository& order_repo,
                   IClockProvider& clock);

    void enqueue(int order_id, int sample_id, int actual_qty, double avg_production_time);
    void tick();
    bool isProducing() const;
    const ProductionJob* getCurrentJob() const;
    int getQueueSize() const;
    std::queue<ProductionJob> getWaitingQueue() const;

private:
    InventoryService& inventory_;
    IOrderRepository& order_repo_;
    IClockProvider& clock_;
    std::unique_ptr<ProductionJob> current_job_;
    std::queue<ProductionJob> waiting_queue_;

    void startNextJob();
};
```

- [ ] `service/ProductionLine.cpp` 생성 후 vcxproj에 추가 (stub)

```cpp
// service/ProductionLine.cpp — stub (Task 8에서 완성)
#include "service/ProductionLine.h"

ProductionLine::ProductionLine(InventoryService& inventory, IOrderRepository& order_repo,
                                IClockProvider& clock)
    : inventory_(inventory), order_repo_(order_repo), clock_(clock) {}

void ProductionLine::enqueue(int order_id, int sample_id,
                              int actual_qty, double avg_production_time) {
    ProductionJob job(order_id, sample_id, actual_qty, avg_production_time, clock_);
    if (!current_job_) current_job_ = std::make_unique<ProductionJob>(job);
    else waiting_queue_.push(job);
}

void ProductionLine::tick() {}  // Task 8에서 구현
bool ProductionLine::isProducing() const { return current_job_ != nullptr; }
const ProductionJob* ProductionLine::getCurrentJob() const { return current_job_.get(); }
int ProductionLine::getQueueSize() const { return static_cast<int>(waiting_queue_.size()); }
std::queue<ProductionJob> ProductionLine::getWaitingQueue() const { return waiting_queue_; }
void ProductionLine::startNextJob() {
    if (!waiting_queue_.empty()) {
        current_job_ = std::make_unique<ProductionJob>(waiting_queue_.front());
        waiting_queue_.pop();
    }
}
```

### Step 7-3: OrderService 구현

- [ ] `service/OrderService.h` 생성 후 vcxproj에 추가

```cpp
// service/OrderService.h
#pragma once
#include "interface/IOrderRepository.h"
#include "interface/ISampleRepository.h"
#include "service/InventoryService.h"
#include "service/ProductionLine.h"

class OrderService {
public:
    OrderService(IOrderRepository& order_repo, ISampleRepository& sample_repo,
                 InventoryService& inventory, ProductionLine& production_line);

    Order* placeOrder(int sample_id, const std::string& customer_name, int quantity);
    bool approveOrder(int order_id);
    bool rejectOrder(int order_id);
    std::vector<Order*> getReservedOrders();
    std::vector<Order*> getAllOrders();

private:
    IOrderRepository& order_repo_;
    ISampleRepository& sample_repo_;
    InventoryService& inventory_;
    ProductionLine& production_line_;
};
```

- [ ] `service/OrderService.cpp` 생성 후 vcxproj에 추가

```cpp
// service/OrderService.cpp
#include "service/OrderService.h"
#include <cmath>

OrderService::OrderService(IOrderRepository& order_repo, ISampleRepository& sample_repo,
                           InventoryService& inventory, ProductionLine& production_line)
    : order_repo_(order_repo), sample_repo_(sample_repo),
      inventory_(inventory), production_line_(production_line) {}

Order* OrderService::placeOrder(int sample_id, const std::string& customer_name, int quantity) {
    if (sample_repo_.findById(sample_id) == nullptr) return nullptr;
    int id = order_repo_.nextId();
    order_repo_.save(Order(id, sample_id, customer_name, quantity));
    return order_repo_.findById(id);
}

bool OrderService::approveOrder(int order_id) {
    Order* order = order_repo_.findById(order_id);
    if (!order || order->getStatus() != OrderStatus::RESERVED) return false;

    int available = inventory_.getAvailableStock(order->getSampleId());

    if (available >= order->getQuantity()) {
        order->setStatus(OrderStatus::CONFIRMED);
    } else {
        order->setStatus(OrderStatus::PRODUCING);
        Sample* sample = sample_repo_.findById(order->getSampleId());
        int shortfall = order->getQuantity() - available;
        int actual_qty = static_cast<int>(
            std::ceil(shortfall / (sample->getYield() * 0.9)));
        production_line_.enqueue(order_id, order->getSampleId(),
                                 actual_qty, sample->getAvgProductionTime());
    }
    return true;
}

bool OrderService::rejectOrder(int order_id) {
    Order* order = order_repo_.findById(order_id);
    if (!order || order->getStatus() != OrderStatus::RESERVED) return false;
    order->setStatus(OrderStatus::REJECTED);
    return true;
}

std::vector<Order*> OrderService::getReservedOrders() {
    return order_repo_.findByStatus(OrderStatus::RESERVED);
}

std::vector<Order*> OrderService::getAllOrders() {
    return order_repo_.findAll();
}
```

- [ ] 빌드 & 실행 → `OrderServiceTest.*` 7개 PASSED 확인

- [ ] 커밋

```
[feat](service): OrderService - 주문 접수/승인/거절 및 재고 기반 상태 결정 구현
```

---

## Task 8: ProductionLine — FIFO 큐 + tick 기반 생산

**Files:**
- Modify: `SampleOrderSystem/service/ProductionLine.cpp`
- Create: `SampleOrderSystem/test/ProductionLineTest.cpp`

### Step 8-1: MockClock 정의 + 테스트 먼저 작성

> `IClockProvider`를 gmock으로 모킹해 실제 시간 대기 없이 시간을 임의로 제어한다.

- [ ] `test/ProductionLineTest.cpp` 생성 후 vcxproj에 추가

```cpp
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

// 시간 mocking용 MockClock
struct MockClock : public IClockProvider {
    MOCK_METHOD(TimePoint, now, (), (const, override));
};

// 테스트용 시간 헬퍼: base에서 N초 후
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
    // 생산시간 1.0초짜리 2개 → 2.0초 경과 후 tick
    EXPECT_CALL(mock_clock, now())
        .WillOnce(Return(t0))                      // enqueue 시 resetLastTick
        .WillOnce(Return(advanceSec(t0, 2.5)));    // tick 시

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
        .WillOnce(Return(t0))                      // job1 resetLastTick
        .WillOnce(Return(t0))                      // job2 resetLastTick
        .WillOnce(Return(advanceSec(t0, 1.5)))     // tick1: job1 완료
        .WillOnce(Return(advanceSec(t0, 1.5)))     // startNextJob → job2 resetLastTick
        .WillOnce(Return(advanceSec(t0, 3.0)));    // tick2: job2 완료

    prod->enqueue(1, 1, 1, 1.0);
    prod->enqueue(2, 1, 1, 1.0);
    prod->tick();  // job1 완료 → job2 시작
    prod->tick();  // job2 완료

    EXPECT_FALSE(prod->isProducing());
    EXPECT_EQ(order_repo->findById(1)->getStatus(), OrderStatus::CONFIRMED);
    EXPECT_EQ(order_repo->findById(2)->getStatus(), OrderStatus::CONFIRMED);
}
```

- [ ] 빌드 → 테스트 실패 확인 (`TickUpdatesInventory`, `ProductionCompleteChanges...` FAIL)

### Step 8-2: ProductionLine tick 완성

- [ ] `service/ProductionLine.cpp`의 `tick()` 구현 수정

```cpp
void ProductionLine::tick() {
    if (!current_job_) return;

    int newly = current_job_->calcNewlyProduced();  // clock은 job 내부에서 호출

    if (newly > 0) {
        current_job_->addProduced(newly);
        inventory_.addActualStock(current_job_->getSampleId(), newly);
    }

    if (current_job_->isComplete()) {
        Order* order = order_repo_.findById(current_job_->getOrderId());
        if (order) order->setStatus(OrderStatus::CONFIRMED);
        current_job_.reset();
        startNextJob();
    }
}

void ProductionLine::startNextJob() {
    if (!waiting_queue_.empty()) {
        current_job_ = std::make_unique<ProductionJob>(waiting_queue_.front());
        current_job_->resetLastTick();  // 새 작업 시작 시점으로 last_tick 초기화
        waiting_queue_.pop();
    }
}
```

- [ ] 빌드 & 실행 → `ProductionLineTest.*` 6개 PASSED 확인

- [ ] 커밋

```
[feat](service): ProductionLine - FIFO 큐 및 tick 기반 생산 시뮬레이션 구현
```

---

## Task 9: ShipmentService — 출고 처리

**Files:**
- Create: `SampleOrderSystem/service/ShipmentService.h / .cpp`
- Create: `SampleOrderSystem/test/ShipmentServiceTest.cpp`

### Step 9-1: 테스트 먼저 작성

- [ ] `test/ShipmentServiceTest.cpp` 생성 후 vcxproj에 추가

```cpp
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
```

- [ ] 빌드 → 컴파일 오류 확인 (ShipmentService 미정의)

### Step 9-2: ShipmentService 구현

- [ ] `service/ShipmentService.h` 생성 후 vcxproj에 추가

```cpp
// service/ShipmentService.h
#pragma once
#include "interface/IOrderRepository.h"
#include "service/InventoryService.h"

class ShipmentService {
public:
    ShipmentService(IOrderRepository& order_repo, InventoryService& inventory);

    bool release(int order_id);
    std::vector<Order*> getConfirmedOrders();

private:
    IOrderRepository& order_repo_;
    InventoryService& inventory_;
};
```

- [ ] `service/ShipmentService.cpp` 생성 후 vcxproj에 추가

```cpp
// service/ShipmentService.cpp
#include "service/ShipmentService.h"

ShipmentService::ShipmentService(IOrderRepository& order_repo, InventoryService& inventory)
    : order_repo_(order_repo), inventory_(inventory) {}

bool ShipmentService::release(int order_id) {
    Order* order = order_repo_.findById(order_id);
    if (!order || order->getStatus() != OrderStatus::CONFIRMED) return false;
    inventory_.subtractActualStock(order->getSampleId(), order->getQuantity());
    order->setStatus(OrderStatus::RELEASE);
    return true;
}

std::vector<Order*> ShipmentService::getConfirmedOrders() {
    return order_repo_.findByStatus(OrderStatus::CONFIRMED);
}
```

- [ ] 빌드 & 실행 → `ShipmentServiceTest.*` 4개 PASSED 확인

- [ ] 커밋

```
[feat](service): ShipmentService - 출고 처리 구현
```

---

## Task 10: View 레이어 — ConsoleUtils (공통 유틸리티)

> View는 stdin/stdout에 의존하므로 단위 테스트 대상 아님. 빌드 통과로 검증.

**Files:**
- Create: `SampleOrderSystem/view/ConsoleUtils.h`
- Create: `SampleOrderSystem/view/ConsoleUtils.cpp`

- [ ] `view/ConsoleUtils.h` 생성 후 vcxproj에 추가

```cpp
// view/ConsoleUtils.h
#pragma once
#include <string>

class ConsoleUtils {
public:
    static void clearScreen();
    static void printHeader(const std::string& title);
    static void printSeparator();
    static int readInt(const std::string& prompt);
    static std::string readString(const std::string& prompt);
    static void pause();
};
```

- [ ] `view/ConsoleUtils.cpp` 생성 후 vcxproj에 추가

```cpp
// view/ConsoleUtils.cpp
#include "view/ConsoleUtils.h"
#include <iostream>
#include <limits>

void ConsoleUtils::clearScreen() { system("cls"); }

void ConsoleUtils::printHeader(const std::string& title) {
    printSeparator();
    std::cout << "  " << title << "\n";
    printSeparator();
}

void ConsoleUtils::printSeparator() {
    std::cout << "========================================\n";
}

int ConsoleUtils::readInt(const std::string& prompt) {
    int val;
    while (true) {
        std::cout << prompt;
        if (std::cin >> val) { std::cin.ignore(); return val; }
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "숫자를 입력하세요.\n";
    }
}

std::string ConsoleUtils::readString(const std::string& prompt) {
    std::string val;
    std::cout << prompt;
    std::getline(std::cin, val);
    return val;
}

void ConsoleUtils::pause() {
    std::cout << "\n계속하려면 Enter를 누르세요...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}
```

- [ ] 빌드 확인 (전체 테스트 PASSED)

- [ ] 커밋

```
[feat](view): ConsoleUtils - 콘솔 공통 유틸리티 구현
```

---

## Task 11: View 레이어 — SampleView (시료 관리 화면)

**Files:**
- Create: `SampleOrderSystem/view/SampleView.h`
- Create: `SampleOrderSystem/view/SampleView.cpp`

- [ ] `view/SampleView.h / .cpp` 생성 후 vcxproj에 추가

```cpp
// view/SampleView.h
#pragma once
#include "service/SampleService.h"
#include "service/InventoryService.h"

class SampleView {
public:
    SampleView(SampleService& sample_svc, InventoryService& inventory);
    void run();

private:
    SampleService& sample_svc_;
    InventoryService& inventory_;
    void registerSample();
    void listSamples();
    void searchSample();
};
```

```cpp
// view/SampleView.cpp
#include "view/SampleView.h"
#include "view/ConsoleUtils.h"
#include <iostream>
#include <cstdio>

SampleView::SampleView(SampleService& sample_svc, InventoryService& inventory)
    : sample_svc_(sample_svc), inventory_(inventory) {}

void SampleView::run() {
    while (true) {
        ConsoleUtils::clearScreen();
        ConsoleUtils::printHeader("시료 관리");
        std::cout << "1. 시료 등록\n2. 시료 조회\n3. 시료 검색\n0. 돌아가기\n";
        int choice = ConsoleUtils::readInt("> ");
        if (choice == 0) return;
        if (choice == 1) registerSample();
        else if (choice == 2) listSamples();
        else if (choice == 3) searchSample();
    }
}

void SampleView::registerSample() {
    ConsoleUtils::printHeader("시료 등록");
    int id = ConsoleUtils::readInt("시료 ID: ");
    std::string name = ConsoleUtils::readString("시료 이름: ");
    int avg_time = ConsoleUtils::readInt("평균 생산시간(초): ");
    int yield_pct = ConsoleUtils::readInt("수율(%): ");

    if (sample_svc_.registerSample(id, name, static_cast<double>(avg_time), yield_pct / 100.0))
        std::cout << "시료가 등록되었습니다.\n";
    else
        std::cout << "이미 존재하는 ID입니다.\n";
    ConsoleUtils::pause();
}

void SampleView::listSamples() {
    ConsoleUtils::printHeader("시료 목록");
    auto samples = sample_svc_.getAllSamples();
    if (samples.empty()) { std::cout << "등록된 시료가 없습니다.\n"; }
    else {
        printf("%-4s %-16s %-12s %-6s %-5s %s\n", "ID", "이름", "평균생산시간", "수율", "재고", "상태");
        ConsoleUtils::printSeparator();
        for (auto* s : samples)
            printf("%-4d %-16s %-12.1f %-6.0f%% %-5d %s\n",
                   s->getId(), s->getName().c_str(),
                   s->getAvgProductionTime(), s->getYield() * 100,
                   inventory_.getActualStock(s->getId()),
                   inventory_.getStockStatus(s->getId()));
    }
    ConsoleUtils::pause();
}

void SampleView::searchSample() {
    ConsoleUtils::printHeader("시료 검색");
    std::string keyword = ConsoleUtils::readString("검색어: ");
    auto results = sample_svc_.searchByName(keyword);
    if (results.empty()) std::cout << "검색 결과 없음.\n";
    else
        for (auto* s : results)
            printf("ID:%-4d 이름:%-16s 수율:%.0f%%\n",
                   s->getId(), s->getName().c_str(), s->getYield() * 100);
    ConsoleUtils::pause();
}
```

- [ ] 빌드 확인 (전체 테스트 PASSED)

- [ ] 커밋

```
[feat](view): SampleView - 시료 등록/조회/검색 화면 구현
```

---

## Task 12: View 레이어 — OrderView (주문 접수/승인/거절 화면)

**Files:**
- Create: `SampleOrderSystem/view/OrderView.h`
- Create: `SampleOrderSystem/view/OrderView.cpp`

- [ ] `view/OrderView.h / .cpp` 생성 후 vcxproj에 추가

```cpp
// view/OrderView.h
#pragma once
#include "service/OrderService.h"
#include "service/SampleService.h"

class OrderView {
public:
    OrderView(OrderService& order_svc, SampleService& sample_svc);
    void run();

private:
    OrderService& order_svc_;
    SampleService& sample_svc_;
    void placeOrder();
    void processApproval();
};
```

```cpp
// view/OrderView.cpp
#include "view/OrderView.h"
#include "view/ConsoleUtils.h"
#include <iostream>
#include <cstdio>

OrderView::OrderView(OrderService& order_svc, SampleService& sample_svc)
    : order_svc_(order_svc), sample_svc_(sample_svc) {}

void OrderView::run() {
    while (true) {
        ConsoleUtils::clearScreen();
        ConsoleUtils::printHeader("주문 관리");
        std::cout << "1. 주문 접수\n2. 주문 승인/거절\n0. 돌아가기\n";
        int choice = ConsoleUtils::readInt("> ");
        if (choice == 0) return;
        if (choice == 1) placeOrder();
        else if (choice == 2) processApproval();
    }
}

void OrderView::placeOrder() {
    ConsoleUtils::printHeader("주문 접수");
    int sample_id = ConsoleUtils::readInt("시료 ID: ");
    std::string customer = ConsoleUtils::readString("고객명: ");
    int qty = ConsoleUtils::readInt("주문 수량: ");
    Order* order = order_svc_.placeOrder(sample_id, customer, qty);
    if (order) printf("주문 접수 완료. 주문 ID: %d (RESERVED)\n", order->getId());
    else std::cout << "존재하지 않는 시료 ID입니다.\n";
    ConsoleUtils::pause();
}

void OrderView::processApproval() {
    ConsoleUtils::printHeader("주문 승인/거절");
    auto reserved = order_svc_.getReservedOrders();
    if (reserved.empty()) { std::cout << "접수된 주문이 없습니다.\n"; ConsoleUtils::pause(); return; }
    printf("%-4s %-16s %-7s %s\n", "ID", "고객명", "시료ID", "수량");
    ConsoleUtils::printSeparator();
    for (auto* o : reserved)
        printf("%-4d %-16s %-7d %d\n", o->getId(), o->getCustomerName().c_str(),
               o->getSampleId(), o->getQuantity());
    int order_id = ConsoleUtils::readInt("\n주문 ID: ");
    std::cout << "1. 승인  2. 거절  0. 취소\n";
    int action = ConsoleUtils::readInt("> ");
    if (action == 1)
        order_svc_.approveOrder(order_id) ? std::cout << "승인 처리되었습니다.\n" : std::cout << "처리 실패.\n";
    else if (action == 2)
        order_svc_.rejectOrder(order_id) ? std::cout << "거절 처리되었습니다.\n" : std::cout << "처리 실패.\n";
    ConsoleUtils::pause();
}
```

- [ ] 빌드 확인 (전체 테스트 PASSED)

- [ ] 커밋

```
[feat](view): OrderView - 주문 접수/승인/거절 화면 구현
```

---

## Task 13: View 레이어 — MonitoringView (모니터링 화면)

> DataMonitor POC 반영: in-memory 상태를 tick 기반으로 읽어 출력.

**Files:**
- Create: `SampleOrderSystem/view/MonitoringView.h`
- Create: `SampleOrderSystem/view/MonitoringView.cpp`

- [ ] `view/MonitoringView.h / .cpp` 생성 후 vcxproj에 추가

```cpp
// view/MonitoringView.h
#pragma once
#include "service/OrderService.h"
#include "service/SampleService.h"
#include "service/InventoryService.h"
#include "service/ProductionLine.h"

class MonitoringView {
public:
    MonitoringView(OrderService& order_svc, SampleService& sample_svc,
                   InventoryService& inventory, ProductionLine& production_line);
    void render();

private:
    OrderService& order_svc_;
    SampleService& sample_svc_;
    InventoryService& inventory_;
    ProductionLine& production_line_;
};
```

```cpp
// view/MonitoringView.cpp
#include "view/MonitoringView.h"
#include "view/ConsoleUtils.h"
#include <iostream>
#include <cstdio>

MonitoringView::MonitoringView(OrderService& order_svc, SampleService& sample_svc,
                                InventoryService& inventory, ProductionLine& production_line)
    : order_svc_(order_svc), sample_svc_(sample_svc),
      inventory_(inventory), production_line_(production_line) {}

void MonitoringView::render() {
    production_line_.tick();
    ConsoleUtils::clearScreen();
    ConsoleUtils::printHeader("모니터링");
    auto all = order_svc_.getAllOrders();
    int cnt[4] = {};
    for (auto* o : all) {
        if (o->getStatus() == OrderStatus::RESERVED)  cnt[0]++;
        if (o->getStatus() == OrderStatus::PRODUCING) cnt[1]++;
        if (o->getStatus() == OrderStatus::CONFIRMED) cnt[2]++;
        if (o->getStatus() == OrderStatus::RELEASE)   cnt[3]++;
    }
    std::cout << "[주문 현황]\n";
    printf("  RESERVED:%d  PRODUCING:%d  CONFIRMED:%d  RELEASE:%d\n\n",
           cnt[0], cnt[1], cnt[2], cnt[3]);

    if (production_line_.isProducing()) {
        auto* job = production_line_.getCurrentJob();
        std::cout << "[현재 생산 중]\n";
        printf("  주문 ID:%d | 시료 ID:%d | 생산량:%d / %d\n\n",
               job->getOrderId(), job->getSampleId(),
               job->getProducedQty(), job->getActualQty());
    }

    std::cout << "[재고 현황]\n";
    printf("%-4s %-16s %-9s %-9s %-9s %s\n", "ID", "이름", "실재재고", "필요재고", "가용재고", "상태");
    ConsoleUtils::printSeparator();
    for (auto* s : sample_svc_.getAllSamples()) {
        printf("%-4d %-16s %-9d %-9d %-9d %s\n",
               s->getId(), s->getName().c_str(),
               inventory_.getActualStock(s->getId()),
               inventory_.getRequiredStock(s->getId()),
               inventory_.getAvailableStock(s->getId()),
               inventory_.getStockStatus(s->getId()));
    }
    ConsoleUtils::pause();
}
```

- [ ] 빌드 확인 (전체 테스트 PASSED)

- [ ] 커밋

```
[feat](view): MonitoringView - 주문/재고/생산 현황 모니터링 화면 구현
```

---

## Task 14: View 레이어 — ProductionView (생산 라인 화면)

**Files:**
- Create: `SampleOrderSystem/view/ProductionView.h`
- Create: `SampleOrderSystem/view/ProductionView.cpp`

- [ ] `view/ProductionView.h / .cpp` 생성 후 vcxproj에 추가

```cpp
// view/ProductionView.h
#pragma once
#include "service/ProductionLine.h"

class ProductionView {
public:
    explicit ProductionView(ProductionLine& production_line);
    void run();

private:
    ProductionLine& production_line_;
};
```

```cpp
// view/ProductionView.cpp
#include "view/ProductionView.h"
#include "view/ConsoleUtils.h"
#include <iostream>
#include <cstdio>

ProductionView::ProductionView(ProductionLine& production_line)
    : production_line_(production_line) {}

void ProductionView::run() {
    production_line_.tick();
    ConsoleUtils::clearScreen();
    ConsoleUtils::printHeader("생산 라인");
    if (production_line_.isProducing()) {
        auto* job = production_line_.getCurrentJob();
        std::cout << "[현재 생산 중]\n";
        printf("  주문 ID:%d | 시료 ID:%d | 생산량:%d / %d\n\n",
               job->getOrderId(), job->getSampleId(),
               job->getProducedQty(), job->getActualQty());
    } else {
        std::cout << "[생산 중인 작업 없음]\n\n";
    }
    std::cout << "[대기 큐] " << production_line_.getQueueSize() << "건\n";
    auto q = production_line_.getWaitingQueue();
    int pos = 1;
    while (!q.empty()) {
        auto job = q.front(); q.pop();
        printf("  %d. 주문ID:%d | 시료ID:%d | 예정:%d개\n",
               pos++, job.getOrderId(), job.getSampleId(), job.getActualQty());
    }
    ConsoleUtils::pause();
}
```

- [ ] 빌드 확인 (전체 테스트 PASSED)

- [ ] 커밋

```
[feat](view): ProductionView - 생산 라인 현황 및 대기 큐 화면 구현
```

---

## Task 15: View 레이어 — ShipmentView (출고 처리 화면)

**Files:**
- Create: `SampleOrderSystem/view/ShipmentView.h`
- Create: `SampleOrderSystem/view/ShipmentView.cpp`

- [ ] `view/ShipmentView.h / .cpp` 생성 후 vcxproj에 추가

```cpp
// view/ShipmentView.h
#pragma once
#include "service/ShipmentService.h"

class ShipmentView {
public:
    explicit ShipmentView(ShipmentService& shipment_svc);
    void run();

private:
    ShipmentService& shipment_svc_;
};
```

```cpp
// view/ShipmentView.cpp
#include "view/ShipmentView.h"
#include "view/ConsoleUtils.h"
#include <iostream>
#include <cstdio>

ShipmentView::ShipmentView(ShipmentService& shipment_svc)
    : shipment_svc_(shipment_svc) {}

void ShipmentView::run() {
    ConsoleUtils::clearScreen();
    ConsoleUtils::printHeader("출고 처리");
    auto confirmed = shipment_svc_.getConfirmedOrders();
    if (confirmed.empty()) {
        std::cout << "출고 가능한 주문이 없습니다.\n";
        ConsoleUtils::pause(); return;
    }
    printf("%-4s %-16s %-7s %s\n", "ID", "고객명", "시료ID", "수량");
    ConsoleUtils::printSeparator();
    for (auto* o : confirmed)
        printf("%-4d %-16s %-7d %d\n", o->getId(), o->getCustomerName().c_str(),
               o->getSampleId(), o->getQuantity());
    int order_id = ConsoleUtils::readInt("\n출고할 주문 ID (0: 취소): ");
    if (order_id == 0) return;
    shipment_svc_.release(order_id)
        ? std::cout << "출고 처리 완료. 상태: RELEASE\n"
        : std::cout << "처리 실패. CONFIRMED 상태인지 확인하세요.\n";
    ConsoleUtils::pause();
}
```

- [ ] 빌드 확인 (전체 테스트 PASSED)

- [ ] 커밋

```
[feat](view): ShipmentView - 출고 처리 화면 구현
```

---

## Task 16: View 레이어 — MainView (메인 메뉴 화면)

**Files:**
- Create: `SampleOrderSystem/view/MainView.h`
- Create: `SampleOrderSystem/view/MainView.cpp`

- [ ] `view/MainView.h / .cpp` 생성 후 vcxproj에 추가

```cpp
// view/MainView.h
#pragma once
#include "service/SampleService.h"
#include "service/OrderService.h"
#include "service/InventoryService.h"
#include "service/ProductionLine.h"
#include "service/ShipmentService.h"

class MainView {
public:
    MainView(SampleService& sample_svc, OrderService& order_svc,
             InventoryService& inventory, ProductionLine& production_line,
             ShipmentService& shipment_svc);
    int promptMainMenu();
    void printSummary();

private:
    SampleService& sample_svc_;
    OrderService& order_svc_;
    InventoryService& inventory_;
    ProductionLine& production_line_;
    ShipmentService& shipment_svc_;
};
```

```cpp
// view/MainView.cpp
#include "view/MainView.h"
#include "view/ConsoleUtils.h"
#include <iostream>

MainView::MainView(SampleService& sample_svc, OrderService& order_svc,
                   InventoryService& inventory, ProductionLine& production_line,
                   ShipmentService& shipment_svc)
    : sample_svc_(sample_svc), order_svc_(order_svc), inventory_(inventory),
      production_line_(production_line), shipment_svc_(shipment_svc) {}

void MainView::printSummary() {
    std::cout << "등록 시료: " << sample_svc_.getAllSamples().size() << "종  "
              << "전체 주문: " << order_svc_.getAllOrders().size() << "건";
    if (production_line_.isProducing()) std::cout << "  [생산 중]";
    std::cout << "\n";
}

int MainView::promptMainMenu() {
    ConsoleUtils::clearScreen();
    ConsoleUtils::printHeader("S-Semi 시료 생산주문관리 시스템");
    printSummary();
    std::cout << "\n1. 시료 관리\n2. 주문 (접수/승인/거절)\n"
                 "3. 모니터링\n4. 출고 처리\n5. 생산 라인\n0. 종료\n";
    return ConsoleUtils::readInt("> ");
}
```

- [ ] 빌드 확인 (전체 테스트 PASSED)

- [ ] 커밋

```
[feat](view): MainView - 메인 메뉴 및 시스템 요약 화면 구현
```

---

## Task 17: AppController + main.cpp 완성

> ConsoleMVC POC 반영: AppController가 모든 services + MainView를 멤버로 소유하며 `run()`에서 tick + 렌더링 + 사용자 입력 → 서비스 호출 루프를 구동한다.

**Files:**
- Create: `SampleOrderSystem/controller/AppController.h`
- Create: `SampleOrderSystem/controller/AppController.cpp`
- Modify: `SampleOrderSystem/main.cpp`

- [ ] `controller/AppController.h` 생성 후 vcxproj에 추가

```cpp
// controller/AppController.h
#pragma once
#include "repository/JsonSampleRepository.h"
#include "repository/JsonOrderRepository.h"
#include "service/RealClock.h"
#include "service/InventoryService.h"
#include "service/SampleService.h"
#include "service/OrderService.h"
#include "service/ProductionLine.h"
#include "service/ShipmentService.h"
#include "view/MainView.h"
#include "view/SampleView.h"
#include "view/OrderView.h"
#include "view/MonitoringView.h"
#include "view/ProductionView.h"
#include "view/ShipmentView.h"

class AppController {
public:
    AppController();
    void run();

private:
    JsonSampleRepository sample_repo_;
    JsonOrderRepository  order_repo_;
    RealClock clock_;
    InventoryService inventory_;
    SampleService    sample_svc_;
    ProductionLine   production_line_;
    OrderService     order_svc_;
    ShipmentService  shipment_svc_;
    MainView main_view_;

    void handleSample();
    void handleOrder();
    void handleMonitoring();
    void handleShipment();
    void handleProduction();
};
```

- [ ] `controller/AppController.cpp` 생성 후 vcxproj에 추가

```cpp
// controller/AppController.cpp
#include "controller/AppController.h"
#include "view/SampleView.h"
#include "view/OrderView.h"
#include "view/MonitoringView.h"
#include "view/ProductionView.h"
#include "view/ShipmentView.h"

AppController::AppController()
    : sample_repo_("data/samples.json"),
      order_repo_("data/orders.json"),
      inventory_(order_repo_, "data/inventory.json"),
      sample_svc_(sample_repo_),
      production_line_(inventory_, order_repo_, clock_),
      order_svc_(order_repo_, sample_repo_, inventory_, production_line_),
      shipment_svc_(order_repo_, inventory_),
      main_view_(sample_svc_, order_svc_, inventory_, production_line_, shipment_svc_) {}

void AppController::run() {
    while (true) {
        production_line_.tick();
        int choice = main_view_.promptMainMenu();
        switch (choice) {
            case 1: handleSample();     break;
            case 2: handleOrder();      break;
            case 3: handleMonitoring(); break;
            case 4: handleShipment();   break;
            case 5: handleProduction(); break;
            case 0: return;
        }
        production_line_.tick();
    }
}

void AppController::handleSample() {
    SampleView v(sample_svc_, inventory_);
    v.run();
}

void AppController::handleOrder() {
    OrderView v(order_svc_, sample_svc_);
    v.run();
}

void AppController::handleMonitoring() {
    MonitoringView v(order_svc_, sample_svc_, inventory_, production_line_);
    v.render();
}

void AppController::handleShipment() {
    ShipmentView v(shipment_svc_);
    v.run();
}

void AppController::handleProduction() {
    ProductionView v(production_line_);
    v.run();
}
```

- [ ] `main.cpp`를 아래와 같이 수정 (테스트 모드 / 앱 모드 분리)

```cpp
// main.cpp
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#ifdef RUN_APP
#include "controller/AppController.h"
#endif

int main(int argc, char* argv[]) {
#ifdef RUN_APP
    AppController app;
    app.run();
    return 0;
#else
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
#endif
}
```

> 테스트 실행: 기본 빌드 → 실행파일 실행  
> 앱 실행: 프로젝트 속성 → C/C++ → 전처리기 → `RUN_APP` 추가 후 빌드

- [ ] 빌드 & 실행 → 전체 테스트 PASSED 확인

- [ ] 통합 시나리오 수동 검증 (`RUN_APP` 추가 후)
    1. 시료 등록 (ID:1, AlphaSi, 생산시간:3초, 수율:90%)
    2. 주문 접수 (시료1, Lab-A, 수량:5)
    3. 주문 승인 → 재고 0 → PRODUCING 전환 확인
    4. 생산 라인 화면에서 진행 상황 확인
    5. 약 21초 대기 → 모니터링에서 CONFIRMED 전환 확인
    6. 출고 처리 → RELEASE 전환 및 재고 차감 확인
    7. 앱 재시작 → 이전 데이터 복원 확인 (JSON 영속성)

- [ ] 커밋

```
[feat](controller): AppController - 메인 루프 및 의존성 조립 완성
[feat](main): 테스트/앱 모드 분리
```

---

## 설계 결정 사항

| 항목 | 결정 | 이유 |
|---|---|---|
| 빌드 시스템 | Visual Studio 2022 vcxproj | 프로젝트 환경 |
| 테스트 프레임워크 | gmock 1.11.0 (NuGet) | 기 설치된 패키지 활용 |
| 생산 시뮬레이션 | tick 폴링 (단일 스레드) | 콘솔 과제 범위 내 단순성 유지 |
| 생산시간 단위 | 초(second) | 데모 시 즉시 확인 가능 |
| 데이터 저장 | 각 Repository/InventoryService 내부 자동 JSON 저장 | DataPersistence POC — 별도 Manager 불필요, write마다 즉시 영속화 |
| JSON 라이브러리 | `nlohmann/json` (NuGet) | 헤더 전용, 설치 간단 |
| 저장 시점 | Repository.save() / InventoryService.addActualStock() 등 write 즉시 | 강제 종료 시에도 마지막 정상 상태 유지 |
| 저장 파일 | `data/samples.json`, `data/orders.json`, `data/inventory.json` | 항목별 분리로 디버깅 용이 |
| View 계층 명칭 | `view/` 폴더, `XxxView` 클래스 | ConsoleMVC POC 정렬 |
| AppController | 모든 서비스 + MainView 소유, run() 메인 루프 | ConsoleMVC POC — Controller → View → Model 의존성 방향 |
| 모니터링 | MonitoringView.render() 호출 시 tick → in-memory 상태 즉시 출력 | DataMonitor POC — polling 없이 요청 시 최신 상태 표시 |
| 인터페이스 선언 | `#define interface struct` | Code_convention.md 준수 |
| 의존성 주입 | 생성자 주입 (레퍼런스) | 테스트 교체 용이, 소유권 명확 |
| 시간 추상화 | `IClockProvider` 인터페이스 + `RealClock` / `MockClock` | TC에서 실제 대기 없이 시간 제어 가능 |
| TDD 순서 | 테스트 → 실패 확인 → 구현 → 통과 확인 | AI_dev_guidelines.md 준수 |
