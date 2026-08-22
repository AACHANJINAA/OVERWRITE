# Architecture

## 목적
이 문서는 `SYSTEM::OVERWRITE`에서 파일과 책임을 나누는 기준입니다.

## 구조 원칙
- C++ 코드, OpenGL 렌더링, 게임 데이터, 문서 책임을 분리합니다.
- 지금 필요한 범위에서만 구조를 확장합니다.
- 1.44MB 목표를 해치지 않도록 대형 외부 에셋 의존을 만들지 않습니다.
- 외부 라이브러리를 기본적으로 도입하지 않고 Windows SDK와 OpenGL 기본 라이브러리만 사용합니다.
- 전체 흐름은 `Manager`가 소유하고, 실제 기능 단위는 `Component`가 수행하는 구조를 기본 방향으로 둡니다.
- `Manager`는 생명주기, 실행 순서, 시스템 간 연결을 담당합니다.
- `Component`는 렌더링, 입력, 오디오, 게임 판정처럼 독립된 기능을 담당합니다.

## 허용 기술 범위
- 창과 입력: Win32 API
- 렌더링: Windows 기본 OpenGL(`opengl32.lib`)
- 텍스트: GDI 또는 직접 작성한 초소형 비트맵 폰트
- 사운드: XAudio2 또는 Windows 기본 사운드 API
- 빌드: Visual Studio/MSVC 또는 Windows SDK 도구

## 금지 기술 범위
- GLFW, SDL, SFML, GLUT 같은 창/입력 프레임워크
- GLEW, GLAD 같은 OpenGL 로더
- ImGui 같은 외부 UI 라이브러리
- Assimp, stb 계열 같은 외부 에셋/이미지 로더
- vcpkg, Conan, NuGet 기반 외부 패키지 추가

## 권장 폴더 책임
- `docs/`: 기획, 작업 규칙, 진행 현황 문서
- `OVERWRITE/0.Engine/`: 진입점, 미리 컴파일된 헤더
- `OVERWRITE/1.Component/`: 렌더링, 입력, 오디오처럼 기능 단위 Component
- `OVERWRITE/2.Manager/`: 싱글톤 Manager 기본 템플릿과 실행 흐름 Manager 구현
- `assets/`: 꼭 필요한 소형 리소스만 보관
- `build/`: 로컬 빌드 산출물

## 대표 코드 책임
- `Main.cpp`: 프로그램 진입점과 메인 루프
- `0.Engine/stdafx.*`: Windows/OpenGL 기본 헤더를 모으는 미리 컴파일된 헤더
- `0.Engine/CJ_MathHelper.h`: 반복 사용되는 수학 공식과 안전 계산 함수
- `2.Manager/Manager.h`: 싱글톤 Manager 기본 템플릿
- `1.Component/Component.h`: 기능 단위 Component 기본 클래스
- `1.Component/RenderTypes.h`: 렌더링과 노드 표시용 소형 구조체
- `2.Manager/WindowManager.*`: Win32 창 생성, 메시지 루프, 종료 흐름
- `1.Component/OpenGlRenderComponent.*`: OpenGL 컨텍스트, 기본 렌더링, 뷰포트
- `1.Component/CameraComponent.*`: 회전, 줌, 뷰 적용
- `DebugSystemManager.*`: STL 노드 목록, 선택, 패치 판정 흐름
- `PatchDatabaseComponent.*`: 버그 설명과 패치 후보 데이터
- `AudioComponent.*`: 경고음과 성공음 재생

## 변경 원칙
- 새 파일은 위 책임에 맞는 위치에 둡니다.
- 같은 성격의 기능이 2개 이상 반복될 때만 작은 공통 함수를 검토합니다.
- 대형 라이브러리, 새 렌더링 엔진, 외부 에셋 묶음은 도입하지 않는 것을 원칙으로 합니다. 정말 필요하면 먼저 1.44MB 목표에 미치는 영향을 문서화하고 사용자 승인을 받습니다.
