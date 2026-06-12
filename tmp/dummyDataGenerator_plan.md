# DummyDataGenerator Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use `superpowers:subagent-driven-development` (recommended) or `superpowers:executing-plans` to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** SampleOrderSystem이 읽을 초기 데이터(`data/samples.json`, `data/orders.json`, `data/inventory.json`)를 JSON 스키마 파일 기반으로 생성하는 독립 C++ 콘솔 도구를 구현한다.

**Architecture:** JSON 스키마 파일(`samples/`) → `SchemaParser`가 파싱 → `Generator`가 랜덤 데이터 생성 → `JsonWriter`(DataMonitor POC 패턴)가 `data/` 폴더에 파일 출력. SampleOrderSystem과 **별도 Visual Studio 프로젝트**로 관리한다.

**Tech Stack:** C++20, Visual Studio 2022, `nlohmann/json` (NuGet), `<random>`, `<filesystem>`

**TDD 규칙:** 모든 구현 Task는 아래 순서를 반드시 따른다.
1. 테스트 작성 (컴파일 실패 상태)
2. 빌드 → 실패 확인
3. 구현 작성
4. 빌드 & 실행 → 테스트 통과 확인
5. 커밋

---

## 파일 구조

```
DummyDataGenerator/
├── DummyDataGenerator.vcxproj   # 별도 Visual Studio 프로젝트
├── main.cpp                     # 진입점: 스키마 파싱 → 생성 → 파일 출력
├── include/
│   └── nlohmann/
│       └── json.hpp             # nlohmann/json 헤더 (NuGet 설치 후 자동 배치)
├── samples/                     # JSON 스키마 정의 파일
│   ├── sample_schema.json       # 시료 스키마
│   ├── order_schema.json        # 주문 스키마
│   └── inventory_schema.json    # 재고 스키마
├── src/
│   ├── schema_parser.hpp / .cpp # SchemaParser + SchemaNode
│   ├── generator.hpp / .cpp     # Generator (SchemaNode → 랜덤 데이터)
│   ├── json_writer.hpp / .cpp   # JsonWriter (DataMonitor POC 패턴)
│   └── main.cpp                 # (또는 프로젝트 루트 main.cpp)
├── data/                        # 출력 디렉토리 (런타임 생성)
│   ├── samples.json
│   ├── orders.json
│   └── inventory.json
└── test/
    ├── SchemaParserTest.cpp
    ├── GeneratorTest.cpp
    └── JsonWriterTest.cpp
```

> vcxproj에 새 파일 추가 시: 프로젝트 우클릭 → 추가 → 기존 항목 또는 새 항목

---

## JSON 스키마 파일 정의

### `samples/sample_schema.json`

```json
{
  "type": "object",
  "properties": {
    "id":   {"type": "integer", "minimum": 1, "maximum": 10},
    "name": {"type": "string", "enum": ["AlphaSi", "BetaSi", "GammaSi", "DeltaSi"]},
    "avg_production_time": {"type": "number", "minimum": 1.0, "maximum": 10.0},
    "yield": {"type": "number", "minimum": 0.7, "maximum": 0.99}
  },
  "required": ["id", "name", "avg_production_time", "yield"]
}
```

### `samples/order_schema.json`

```json
{
  "type": "object",
  "properties": {
    "id":            {"type": "integer", "minimum": 1, "maximum": 100},
    "sample_id":     {"type": "integer", "minimum": 1, "maximum": 10},
    "customer_name": {"type": "string", "enum": ["Lab-A", "Lab-B", "Lab-C", "Lab-D", "Lab-E"]},
    "quantity":      {"type": "integer", "minimum": 1, "maximum": 50},
    "status":        {"type": "string", "enum": ["RESERVED", "CONFIRMED", "PRODUCING", "RELEASE", "REJECTED"]}
  },
  "required": ["id", "sample_id", "customer_name", "quantity", "status"]
}
```

### `samples/inventory_schema.json`

```json
{
  "type": "object",
  "properties": {
    "sample_id": {"type": "integer", "minimum": 1, "maximum": 10},
    "stock":     {"type": "integer", "minimum": 0, "maximum": 500}
  },
  "required": ["sample_id", "stock"]
}
```

---

## Task 1: SchemaNode + SchemaParser

**Files:**
- Create: `DummyDataGenerator/src/schema_parser.hpp`
- Create: `DummyDataGenerator/src/schema_parser.cpp`
- Create: `DummyDataGenerator/test/SchemaParserTest.cpp`

### Step 1-1: 테스트 먼저 작성

- [ ] `test/SchemaParserTest.cpp` 생성 후 vcxproj에 추가

```cpp
// test/SchemaParserTest.cpp
#include <gtest/gtest.h>
#include "src/schema_parser.hpp"
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

// 테스트용 임시 스키마 파일 생성 헬퍼
static std::string writeTemp(const std::string& name, const std::string& content) {
    fs::create_directories("test_schemas_tmp");
    std::string path = "test_schemas_tmp/" + name;
    std::ofstream(path) << content;
    return path;
}

class SchemaParserTest : public ::testing::Test {
protected:
    void TearDown() override {
        fs::remove_all("test_schemas_tmp");
    }
};

TEST_F(SchemaParserTest, ParseIntegerWithMinMax) {
    auto path = writeTemp("int.json", R"({
        "type": "integer", "minimum": 1, "maximum": 10
    })");
    auto node = SchemaParser::parseFromSchema(path);
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->type, SchemaType::Integer);
    EXPECT_EQ(node->minimum, 1.0);
    EXPECT_EQ(node->maximum, 10.0);
}

TEST_F(SchemaParserTest, ParseNumberWithMinMax) {
    auto path = writeTemp("num.json", R"({
        "type": "number", "minimum": 0.7, "maximum": 0.99
    })");
    auto node = SchemaParser::parseFromSchema(path);
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->type, SchemaType::Number);
    EXPECT_DOUBLE_EQ(node->minimum, 0.7);
    EXPECT_DOUBLE_EQ(node->maximum, 0.99);
}

TEST_F(SchemaParserTest, ParseStringEnum) {
    auto path = writeTemp("str.json", R"({
        "type": "string", "enum": ["AlphaSi", "BetaSi"]
    })");
    auto node = SchemaParser::parseFromSchema(path);
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->type, SchemaType::String);
    ASSERT_EQ(node->enumValues.size(), 2u);
    EXPECT_EQ(node->enumValues[0], "AlphaSi");
}

TEST_F(SchemaParserTest, ParseObjectProperties) {
    auto path = writeTemp("obj.json", R"({
        "type": "object",
        "properties": {
            "id":   {"type": "integer", "minimum": 1, "maximum": 5},
            "name": {"type": "string", "enum": ["A", "B"]}
        },
        "required": ["id", "name"]
    })");
    auto node = SchemaParser::parseFromSchema(path);
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->type, SchemaType::Object);
    EXPECT_EQ(node->properties.size(), 2u);
    EXPECT_EQ(node->required.size(), 2u);
    ASSERT_NE(node->properties.count("id"), 0u);
    EXPECT_EQ(node->properties.at("id")->type, SchemaType::Integer);
}

TEST_F(SchemaParserTest, ParseMissingFileReturnsNull) {
    auto node = SchemaParser::parseFromSchema("nonexistent.json");
    EXPECT_EQ(node, nullptr);
}
```

- [ ] 빌드 → 컴파일 오류 확인 (SchemaParser 미정의)

### Step 1-2: SchemaNode + SchemaParser 구현

- [ ] `src/schema_parser.hpp` 생성 후 vcxproj에 추가

```cpp
// src/schema_parser.hpp
#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>

enum class SchemaType {
    String,
    Integer,
    Number,
    Boolean,
    Array,
    Object,
    Unknown
};

struct SchemaNode {
    SchemaType type = SchemaType::Unknown;

    // String
    int         minLength  = 0;
    int         maxLength  = 256;
    std::string format;                       // e.g. "date-time"
    std::vector<std::string> enumValues;      // enum 제약

    // Integer / Number
    double minimum = 0.0;
    double maximum = 0.0;

    // Array
    int minItems = 0;
    int maxItems = 10;
    std::shared_ptr<SchemaNode> items;        // 배열 원소 스키마

    // Object
    std::map<std::string, std::shared_ptr<SchemaNode>> properties;
    std::vector<std::string> required;

    // 공통
    bool nullable = false;
};

class SchemaParser {
public:
    // JSON 스키마 파일을 파싱해 SchemaNode 트리 반환. 실패 시 nullptr.
    static std::shared_ptr<SchemaNode> parseFromSchema(const std::string& filepath);

    // 샘플 JSON 파일을 보고 스키마를 추론. 단순 타입 추론만 지원.
    static std::shared_ptr<SchemaNode> inferFromSample(const std::string& filepath);
};
```

- [ ] `src/schema_parser.cpp` 생성 후 vcxproj에 추가

```cpp
// src/schema_parser.cpp
#include "src/schema_parser.hpp"
#include <nlohmann/json.hpp>
#include <fstream>

using json = nlohmann::json;

static SchemaType strToType(const std::string& t) {
    if (t == "string")  return SchemaType::String;
    if (t == "integer") return SchemaType::Integer;
    if (t == "number")  return SchemaType::Number;
    if (t == "boolean") return SchemaType::Boolean;
    if (t == "array")   return SchemaType::Array;
    if (t == "object")  return SchemaType::Object;
    return SchemaType::Unknown;
}

static std::shared_ptr<SchemaNode> parseNode(const json& j) {
    auto node = std::make_shared<SchemaNode>();

    if (j.contains("type"))
        node->type = strToType(j["type"].get<std::string>());

    // String
    if (j.contains("enum"))
        for (auto& v : j["enum"])
            node->enumValues.push_back(v.get<std::string>());
    if (j.contains("minLength")) node->minLength = j["minLength"].get<int>();
    if (j.contains("maxLength")) node->maxLength = j["maxLength"].get<int>();
    if (j.contains("format"))    node->format    = j["format"].get<std::string>();

    // Number / Integer
    if (j.contains("minimum")) node->minimum = j["minimum"].get<double>();
    if (j.contains("maximum")) node->maximum = j["maximum"].get<double>();

    // Array
    if (j.contains("minItems")) node->minItems = j["minItems"].get<int>();
    if (j.contains("maxItems")) node->maxItems = j["maxItems"].get<int>();
    if (j.contains("items"))    node->items    = parseNode(j["items"]);

    // Object
    if (j.contains("properties"))
        for (auto& [key, val] : j["properties"].items())
            node->properties[key] = parseNode(val);
    if (j.contains("required"))
        for (auto& r : j["required"])
            node->required.push_back(r.get<std::string>());

    if (j.contains("nullable")) node->nullable = j["nullable"].get<bool>();

    return node;
}

std::shared_ptr<SchemaNode> SchemaParser::parseFromSchema(const std::string& filepath) {
    std::ifstream f(filepath);
    if (!f.is_open()) return nullptr;
    json j;
    try { f >> j; }
    catch (...) { return nullptr; }
    return parseNode(j);
}

std::shared_ptr<SchemaNode> SchemaParser::inferFromSample(const std::string& filepath) {
    std::ifstream f(filepath);
    if (!f.is_open()) return nullptr;
    json j;
    try { f >> j; }
    catch (...) { return nullptr; }

    // 배열이면 첫 원소에서 타입 추론
    if (j.is_array() && !j.empty()) j = j[0];

    auto node = std::make_shared<SchemaNode>();
    node->type = SchemaType::Object;
    for (auto& [key, val] : j.items()) {
        auto child = std::make_shared<SchemaNode>();
        if (val.is_string())      child->type = SchemaType::String;
        else if (val.is_number_integer()) child->type = SchemaType::Integer;
        else if (val.is_number_float())   child->type = SchemaType::Number;
        else if (val.is_boolean())        child->type = SchemaType::Boolean;
        else if (val.is_array())          child->type = SchemaType::Array;
        else if (val.is_object())         child->type = SchemaType::Object;
        node->properties[key] = child;
        node->required.push_back(key);
    }
    return node;
}
```

- [ ] 빌드 & 실행 → `SchemaParserTest.*` 5개 PASSED 확인

- [ ] 커밋

```
[feat](parser): SchemaNode 구조체 및 SchemaParser JSON 스키마 파싱 구현
```

---

## Task 2: Generator — SchemaNode → 랜덤 데이터 생성

**Files:**
- Create: `DummyDataGenerator/src/generator.hpp`
- Create: `DummyDataGenerator/src/generator.cpp`
- Create: `DummyDataGenerator/test/GeneratorTest.cpp`

### Step 2-1: 테스트 먼저 작성

- [ ] `test/GeneratorTest.cpp` 생성 후 vcxproj에 추가

```cpp
// test/GeneratorTest.cpp
#include <gtest/gtest.h>
#include "src/generator.hpp"
#include "src/schema_parser.hpp"

TEST(GeneratorTest, GenerateIntegerInRange) {
    auto node = std::make_shared<SchemaNode>();
    node->type    = SchemaType::Integer;
    node->minimum = 1.0;
    node->maximum = 5.0;

    Generator gen(42);  // 고정 seed
    for (int i = 0; i < 20; ++i) {
        auto val = gen.generate(node);
        ASSERT_TRUE(val.is_number_integer());
        int v = val.get<int>();
        EXPECT_GE(v, 1);
        EXPECT_LE(v, 5);
    }
}

TEST(GeneratorTest, GenerateNumberInRange) {
    auto node = std::make_shared<SchemaNode>();
    node->type    = SchemaType::Number;
    node->minimum = 0.7;
    node->maximum = 0.99;

    Generator gen(42);
    for (int i = 0; i < 20; ++i) {
        auto val = gen.generate(node);
        ASSERT_TRUE(val.is_number_float());
        double v = val.get<double>();
        EXPECT_GE(v, 0.7);
        EXPECT_LE(v, 0.99);
    }
}

TEST(GeneratorTest, GenerateStringFromEnum) {
    auto node = std::make_shared<SchemaNode>();
    node->type       = SchemaType::String;
    node->enumValues = {"AlphaSi", "BetaSi", "GammaSi"};

    Generator gen(42);
    for (int i = 0; i < 20; ++i) {
        auto val = gen.generate(node);
        ASSERT_TRUE(val.is_string());
        std::string s = val.get<std::string>();
        EXPECT_TRUE(s == "AlphaSi" || s == "BetaSi" || s == "GammaSi");
    }
}

TEST(GeneratorTest, GenerateObjectHasRequiredFields) {
    auto id_node = std::make_shared<SchemaNode>();
    id_node->type    = SchemaType::Integer;
    id_node->minimum = 1.0;
    id_node->maximum = 10.0;

    auto name_node = std::make_shared<SchemaNode>();
    name_node->type       = SchemaType::String;
    name_node->enumValues = {"AlphaSi", "BetaSi"};

    auto obj = std::make_shared<SchemaNode>();
    obj->type = SchemaType::Object;
    obj->properties["id"]   = id_node;
    obj->properties["name"] = name_node;
    obj->required = {"id", "name"};

    Generator gen(42);
    auto val = gen.generate(obj);
    ASSERT_TRUE(val.is_object());
    EXPECT_TRUE(val.contains("id"));
    EXPECT_TRUE(val.contains("name"));
}

TEST(GeneratorTest, GenerateMultipleRecords) {
    auto node = std::make_shared<SchemaNode>();
    node->type    = SchemaType::Integer;
    node->minimum = 1.0;
    node->maximum = 100.0;

    Generator gen(0);
    auto records = gen.generateMany(node, 10);
    EXPECT_EQ(records.size(), 10u);
}

TEST(GeneratorTest, GenerateBooleanReturnsBool) {
    auto node = std::make_shared<SchemaNode>();
    node->type = SchemaType::Boolean;

    Generator gen(42);
    auto val = gen.generate(node);
    EXPECT_TRUE(val.is_boolean());
}
```

- [ ] 빌드 → 컴파일 오류 확인 (Generator 미정의)

### Step 2-2: Generator 구현

- [ ] `src/generator.hpp` 생성 후 vcxproj에 추가

```cpp
// src/generator.hpp
#pragma once
#include <vector>
#include <random>
#include <memory>
#include <nlohmann/json.hpp>
#include "src/schema_parser.hpp"

using json = nlohmann::json;

class Generator {
public:
    explicit Generator(unsigned int seed = std::random_device{}());

    // SchemaNode 하나를 기반으로 랜덤 JSON 값 생성
    json generate(const std::shared_ptr<SchemaNode>& node);

    // count개 JSON 레코드 생성 (node가 Object 타입인 경우 json array)
    std::vector<json> generateMany(const std::shared_ptr<SchemaNode>& node, int count);

private:
    std::mt19937 rng_;

    json generateString(const SchemaNode& node);
    json generateInteger(const SchemaNode& node);
    json generateNumber(const SchemaNode& node);
    json generateBoolean();
    json generateArray(const SchemaNode& node);
    json generateObject(const SchemaNode& node);
};
```

- [ ] `src/generator.cpp` 생성 후 vcxproj에 추가

```cpp
// src/generator.cpp
#include "src/generator.hpp"
#include <stdexcept>
#include <algorithm>

Generator::Generator(unsigned int seed) : rng_(seed) {}

json Generator::generate(const std::shared_ptr<SchemaNode>& node) {
    if (!node) return json(nullptr);
    switch (node->type) {
        case SchemaType::String:  return generateString(*node);
        case SchemaType::Integer: return generateInteger(*node);
        case SchemaType::Number:  return generateNumber(*node);
        case SchemaType::Boolean: return generateBoolean();
        case SchemaType::Array:   return generateArray(*node);
        case SchemaType::Object:  return generateObject(*node);
        default:                  return json(nullptr);
    }
}

std::vector<json> Generator::generateMany(const std::shared_ptr<SchemaNode>& node, int count) {
    std::vector<json> result;
    result.reserve(count);
    for (int i = 0; i < count; ++i)
        result.push_back(generate(node));
    return result;
}

json Generator::generateString(const SchemaNode& node) {
    if (!node.enumValues.empty()) {
        std::uniform_int_distribution<size_t> dist(0, node.enumValues.size() - 1);
        return node.enumValues[dist(rng_)];
    }
    // 임의 영문 소문자 문자열 생성
    int len = std::uniform_int_distribution<int>(
        std::max(1, node.minLength),
        std::max(1, node.maxLength > 0 ? node.maxLength : 16))(rng_);
    std::string s;
    s.reserve(len);
    std::uniform_int_distribution<int> ch('a', 'z');
    for (int i = 0; i < len; ++i)
        s += static_cast<char>(ch(rng_));
    return s;
}

json Generator::generateInteger(const SchemaNode& node) {
    int lo = static_cast<int>(node.minimum);
    int hi = static_cast<int>(node.maximum);
    if (lo > hi) std::swap(lo, hi);
    if (lo == hi) return lo;
    return std::uniform_int_distribution<int>(lo, hi)(rng_);
}

json Generator::generateNumber(const SchemaNode& node) {
    double lo = node.minimum;
    double hi = node.maximum;
    if (lo > hi) std::swap(lo, hi);
    if (lo == hi) return lo;
    return std::uniform_real_distribution<double>(lo, hi)(rng_);
}

json Generator::generateBoolean() {
    return std::bernoulli_distribution(0.5)(rng_);
}

json Generator::generateArray(const SchemaNode& node) {
    int count = std::uniform_int_distribution<int>(node.minItems,
        std::max(node.minItems, node.maxItems))(rng_);
    json arr = json::array();
    if (node.items) {
        for (int i = 0; i < count; ++i)
            arr.push_back(generate(node.items));
    }
    return arr;
}

json Generator::generateObject(const SchemaNode& node) {
    json obj = json::object();
    // required 필드 우선 생성
    for (const auto& key : node.required) {
        auto it = node.properties.find(key);
        if (it != node.properties.end())
            obj[key] = generate(it->second);
    }
    // optional 필드 (required에 없는 것)
    for (const auto& [key, child] : node.properties) {
        if (obj.contains(key)) continue;
        obj[key] = generate(child);
    }
    return obj;
}
```

- [ ] 빌드 & 실행 → `GeneratorTest.*` 6개 PASSED 확인

- [ ] 커밋

```
[feat](generator): Generator - SchemaNode 기반 랜덤 JSON 데이터 생성 구현
```

---

## Task 3: JsonWriter — DataMonitor POC 패턴으로 파일 출력

**Files:**
- Create: `DummyDataGenerator/src/json_writer.hpp`
- Create: `DummyDataGenerator/src/json_writer.cpp`
- Create: `DummyDataGenerator/test/JsonWriterTest.cpp`

### Step 3-1: 테스트 먼저 작성

- [ ] `test/JsonWriterTest.cpp` 생성 후 vcxproj에 추가

```cpp
// test/JsonWriterTest.cpp
#include <gtest/gtest.h>
#include "src/json_writer.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>

using json = nlohmann::json;
namespace fs = std::filesystem;

class JsonWriterTest : public ::testing::Test {
protected:
    const std::string out_dir = "test_writer_tmp";
    void TearDown() override { fs::remove_all(out_dir); }
};

TEST_F(JsonWriterTest, WriteCreatesFile) {
    std::vector<json> records = {
        {{"id", 1}, {"name", "AlphaSi"}},
        {{"id", 2}, {"name", "BetaSi"}}
    };
    JsonWriter writer;
    writer.write(records, out_dir + "/samples.json");
    EXPECT_TRUE(fs::exists(out_dir + "/samples.json"));
}

TEST_F(JsonWriterTest, WriteContentIsValidJson) {
    std::vector<json> records = {{{"id", 1}, {"val", 3.14}}};
    JsonWriter writer;
    writer.write(records, out_dir + "/out.json");

    std::ifstream f(out_dir + "/out.json");
    json loaded; f >> loaded;
    ASSERT_TRUE(loaded.is_array());
    EXPECT_EQ(loaded.size(), 1u);
    EXPECT_EQ(loaded[0]["id"].get<int>(), 1);
}

TEST_F(JsonWriterTest, WriteInventoryAsObject) {
    // inventory는 {"1": 100, "2": 50} 형태
    json inv_obj = {{"1", 100}, {"2", 50}};
    JsonWriter writer;
    writer.writeObject(inv_obj, out_dir + "/inventory.json");

    std::ifstream f(out_dir + "/inventory.json");
    json loaded; f >> loaded;
    ASSERT_TRUE(loaded.is_object());
    EXPECT_EQ(loaded["1"].get<int>(), 100);
}

TEST_F(JsonWriterTest, WriteCreatesParentDirectory) {
    std::vector<json> records = {{{"x", 1}}};
    JsonWriter writer;
    writer.write(records, out_dir + "/nested/dir/out.json");
    EXPECT_TRUE(fs::exists(out_dir + "/nested/dir/out.json"));
}

TEST_F(JsonWriterTest, WriteEmptyArrayCreatesEmptyJson) {
    std::vector<json> records;
    JsonWriter writer;
    writer.write(records, out_dir + "/empty.json");

    std::ifstream f(out_dir + "/empty.json");
    json loaded; f >> loaded;
    EXPECT_TRUE(loaded.is_array());
    EXPECT_EQ(loaded.size(), 0u);
}
```

- [ ] 빌드 → 컴파일 오류 확인 (JsonWriter 미정의)

### Step 3-2: JsonWriter 구현

> DataMonitor POC의 `JsonWriter::write(records, dir)` 패턴을 따른다.

- [ ] `src/json_writer.hpp` 생성 후 vcxproj에 추가

```cpp
// src/json_writer.hpp
#pragma once
#include <vector>
#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class JsonWriter {
public:
    // records(json array)를 filePath에 저장. 상위 디렉토리가 없으면 생성.
    void write(const std::vector<json>& records, const std::string& filePath);

    // 단일 JSON object를 filePath에 저장 (inventory.json 등 객체 형태용)
    void writeObject(const json& obj, const std::string& filePath);

private:
    void ensureParentDir(const std::string& filePath) const;
};
```

- [ ] `src/json_writer.cpp` 생성 후 vcxproj에 추가

```cpp
// src/json_writer.cpp
#include "src/json_writer.hpp"
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

void JsonWriter::ensureParentDir(const std::string& filePath) const {
    auto parent = fs::path(filePath).parent_path();
    if (!parent.empty())
        fs::create_directories(parent);
}

void JsonWriter::write(const std::vector<json>& records, const std::string& filePath) {
    ensureParentDir(filePath);
    json arr = json::array();
    for (const auto& r : records)
        arr.push_back(r);
    std::ofstream(filePath) << arr.dump(2);
}

void JsonWriter::writeObject(const json& obj, const std::string& filePath) {
    ensureParentDir(filePath);
    std::ofstream(filePath) << obj.dump(2);
}
```

- [ ] 빌드 & 실행 → `JsonWriterTest.*` 5개 PASSED 확인

- [ ] 커밋

```
[feat](writer): JsonWriter - DataMonitor POC 패턴으로 JSON 파일 출력 구현
```

---

## Task 4: 통합 — main.cpp 완성

**Files:**
- Modify: `DummyDataGenerator/main.cpp`

> 이 태스크는 모든 컴포넌트를 조립해 실제 데이터 파일을 생성한다. 유닛 테스트 없음; 출력 파일을 수동 검증한다.

- [ ] `main.cpp`를 아래와 같이 작성

```cpp
// main.cpp
#include <gtest/gtest.h>
#include "src/schema_parser.hpp"
#include "src/generator.hpp"
#include "src/json_writer.hpp"
#include <iostream>

#ifdef RUN_APP

// 중복 ID 없이 unique_id를 순차 할당하는 헬퍼
static std::vector<json> generateWithUniqueId(
        Generator& gen,
        const std::shared_ptr<SchemaNode>& schema,
        int count) {
    auto records = gen.generateMany(schema, count);
    for (int i = 0; i < static_cast<int>(records.size()); ++i)
        if (records[i].contains("id"))
            records[i]["id"] = i + 1;
    return records;
}

int main(int argc, char* argv[]) {
    const std::string schema_dir = "samples";
    const std::string out_dir    = "data";

    Generator  gen;   // random seed
    JsonWriter writer;

    // ── samples.json ─────────────────────────────
    auto sample_schema = SchemaParser::parseFromSchema(schema_dir + "/sample_schema.json");
    if (!sample_schema) { std::cerr << "sample_schema.json 파싱 실패\n"; return 1; }
    // 시료 ID는 1~10 고정 (enum과 무관하게 4개 시료만 생성)
    auto samples = generateWithUniqueId(gen, sample_schema, 4);
    // 이름이 겹치지 않도록 enum 순서대로 지정
    std::vector<std::string> names = {"AlphaSi", "BetaSi", "GammaSi", "DeltaSi"};
    for (int i = 0; i < static_cast<int>(samples.size()) && i < static_cast<int>(names.size()); ++i)
        samples[i]["name"] = names[i];
    writer.write(samples, out_dir + "/samples.json");
    std::cout << "[OK] " << out_dir << "/samples.json (" << samples.size() << "개)\n";

    // ── orders.json ──────────────────────────────
    auto order_schema = SchemaParser::parseFromSchema(schema_dir + "/order_schema.json");
    if (!order_schema) { std::cerr << "order_schema.json 파싱 실패\n"; return 1; }
    // sample_id는 실제 생성된 시료 ID(1~4) 범위로 제한
    if (order_schema->properties.count("sample_id"))
        order_schema->properties["sample_id"]->maximum = static_cast<double>(samples.size());
    auto orders = generateWithUniqueId(gen, order_schema, 10);
    writer.write(orders, out_dir + "/orders.json");
    std::cout << "[OK] " << out_dir << "/orders.json (" << orders.size() << "개)\n";

    // ── inventory.json ───────────────────────────
    auto inv_schema = SchemaParser::parseFromSchema(schema_dir + "/inventory_schema.json");
    if (!inv_schema) { std::cerr << "inventory_schema.json 파싱 실패\n"; return 1; }
    json inventory = json::object();
    for (const auto& s : samples) {
        int sid = s["id"].get<int>();
        auto inv_rec = gen.generate(inv_schema);
        inventory[std::to_string(sid)] = inv_rec.contains("stock")
            ? inv_rec["stock"].get<int>()
            : 0;
    }
    writer.writeObject(inventory, out_dir + "/inventory.json");
    std::cout << "[OK] " << out_dir << "/inventory.json\n";

    std::cout << "\n데이터 생성 완료. '" << out_dir << "/' 폴더를 SampleOrderSystem/data/ 에 복사하세요.\n";
    return 0;
}

#else

int main(int argc, char* argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

#endif
```

> 테스트 실행: 기본 빌드 → 실행  
> 데이터 생성: 프로젝트 속성 → C/C++ → 전처리기 → `RUN_APP` 추가 후 빌드 & 실행

- [ ] 빌드 & 실행 (테스트 모드) → 전체 테스트 PASSED 확인

- [ ] `RUN_APP` 추가 후 빌드 & 실행 → 수동 검증
    1. `data/samples.json` — 4개 레코드, id 1~4, name은 AlphaSi/BetaSi/GammaSi/DeltaSi
    2. `data/orders.json` — 10개 레코드, sample_id 범위 1~4, status 값 유효
    3. `data/inventory.json` — 시료 ID 1~4 키, 각 값 0~500 범위
    4. 생성된 파일을 `SampleOrderSystem/data/` 에 복사 후 앱 실행 → 데이터 로드 확인

- [ ] 커밋

```
[feat](main): 스키마 기반 초기 데이터 생성 통합 진입점 완성
```

---

## 설계 결정 사항

| 항목 | 결정 | 이유 |
|---|---|---|
| 프로젝트 분리 | 별도 VS 프로젝트 (`DummyDataGenerator.vcxproj`) | SampleOrderSystem 빌드에 영향 없음 |
| JSON 라이브러리 | `nlohmann/json` (NuGet) | 헤더 전용, SampleOrderSystem과 동일 선택 |
| SchemaParser 방식 | JSON 스키마 파일 파싱 + 샘플 추론 2가지 제공 | DummyDataGenerator POC 패턴 그대로 |
| SchemaNode 타입 | String / Integer / Number / Boolean / Array / Object | DummyDataGenerator POC `SchemaType` enum 준수 |
| Generator seed | 기본 `std::random_device`, 테스트 시 고정 seed | 재현 가능한 단위 테스트 + 운영 시 랜덤성 |
| JsonWriter 패턴 | `write(records, filePath)` + `writeObject(obj, filePath)` | DataMonitor POC `JsonWriter::write(records, dir)` 확장 |
| inventory.json 형태 | `{"1": 100, "2": 50}` (key: sample_id 문자열) | SampleOrderSystem `InventoryService::load()` 포맷과 일치 |
| 중복 ID 방지 | `generateWithUniqueId()` 헬퍼로 순차 ID 재할당 | 스키마 범위 랜덤 → 충돌 가능성 제거 |
| 출력 경로 | `data/` 폴더 (런타임 생성) | SampleOrderSystem `data/` 와 동일 구조 |
| TDD 순서 | 테스트 → 실패 확인 → 구현 → 통과 확인 | AI_dev_guidelines.md 준수 |
