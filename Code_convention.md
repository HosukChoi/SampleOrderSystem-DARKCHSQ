# Code Convention

## Naming

| 대상 | 방식 | 예시 |
|------|------|------|
| 클래스 / 구조체 | PascalCase | `CarValidator`, `CarConfig` |
| 인터페이스 | `I` 접두사 + PascalCase | `IComponent`, `IEngine`, `IBrakeSystem` |
| 메서드 / 함수 | camelCase | `isValid()`, `getFailReason()`, `printMenu()` |
| 멤버 변수 | snake_case | `car_type_`, `fail_reason_`, `current_step_` |

## Interface 정의 규칙

인터페이스를 선언하는 헤더 파일 상단에 `#define interface struct` 를 정의하여  
`interface IXxx` 문법으로 인터페이스임을 명시적으로 표현한다.

```cpp
// IComponent.h
#pragma once

#define interface struct

interface IComponent {
    virtual ~IComponent() = default;
    virtual int getId() const = 0;
    virtual const char* getName() const = 0;
};
```

```cpp
// IEngine.h
#pragma once

#include "IComponent.h"

interface IEngine : public IComponent {
    // 엔진 고유 메서드 확장 가능
};
```
