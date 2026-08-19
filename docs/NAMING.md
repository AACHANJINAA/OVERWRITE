# Naming Guide

## 목적
이 문서는 `SYSTEM::OVERWRITE` C++ 코드에서 사용할 변수, 함수, 클래스, 구조체 이름 규칙을 정의합니다.

## 기본 규칙
- 이름은 역할이 드러나게 작성합니다.
- 축약어는 의미가 분명한 경우에만 사용합니다.
- 같은 종류의 이름은 같은 규칙을 유지합니다.

## 변수
- 일반 변수는 소문자로 작성하고 맨 앞에 `_`를 붙입니다.
- 합성어 변수는 소문자와 `_`를 사용합니다.

예시:
```cpp
int _count;
float _engine_temperature;
HWND _main_window;
```

## bool 변수
- `bool` 변수는 `is` 단어를 포함합니다.
- 일반 변수 규칙과 같이 맨 앞에 `_`를 붙이고 소문자로 작성합니다.

예시:
```cpp
bool _is_running;
bool _is_selected;
bool _is_overheated;
```

## 함수
- 함수는 첫 글자를 대문자로 작성합니다.
- 합성어 함수는 각 단어의 시작을 대문자로 작성합니다.

예시:
```cpp
void RenderFrame();
bool CreateOpenGlContext();
void ResizeViewport();
```

## 클래스
- 클래스 이름은 모두 대문자로 작성합니다.
- 합성어도 모두 대문자로 작성합니다.

예시:
```cpp
class RENDERER;
class PLATFORMWINDOW;
class DEBUGSYSTEM;
```

## 구조체
- 구조체 이름도 클래스와 같이 모두 대문자로 작성합니다.
- 합성어도 모두 대문자로 작성합니다.

예시:
```cpp
struct STLNODE;
struct PATCHDATA;
```

## 멤버 변수
- 멤버 변수는 소문자로 작성합니다.
- 합성어일 경우 단어 사이에 `_`를 추가합니다.
- 일반 변수와 달리 맨 앞 `_`는 붙이지 않습니다.

예시:
```cpp
class DEBUGSYSTEM
{
private:
    int repair_count;
    bool is_system_stable;
};
```

## 멤버 함수
- 멤버 함수는 소문자로 작성합니다.
- 합성어일 경우 단어 사이에 `_`를 추가합니다.

예시:
```cpp
class DEBUGSYSTEM
{
public:
    void update_nodes();
    bool apply_patch();
    int get_repair_count() const;
};
```

## 우선순위
기존 코드와 이 문서가 다를 경우 새로 작성하는 코드는 이 문서를 우선합니다.
기존 코드 수정은 관련 task를 진행할 때 필요한 범위 안에서만 반영합니다.
