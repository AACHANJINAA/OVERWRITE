# SYSTEM::OVERWRITE - OpenGL STL Debug Console

## 목적
이 폴더는 `SYSTEM::OVERWRITE` 게임을 실제 제작 가능한 프로토타입으로 좁히기 위한 MD 기반 기획/작업 문서 모음입니다.

원본 콘셉트의 핵심인 우주선 함교, 네온 와이어프레임 3D 콘솔, STL 컨테이너 버그 수리 퍼즐은 유지하되, `TPSProject` 문서에서 검증된 방식인 작은 단위 task, 명확한 범위 제한, 구현 후 수동 확인 흐름만 가져왔습니다.

## 문서 구성
- `docs/Game_GDD.md`: 게임 전체 기획서
- `docs/STL_Problem.md`: STL 버그 퍼즐 문제은행
- `docs/TASK_BREAKDOWN.md`: 1차 프로토타입 구현 순서
- `docs/ARCHITECTURE.md`: 폴더/파일 책임 기준
- `docs/STYLEGUIDE.md`: C++/OpenGL/에셋/문서 규칙
- `docs/WORKFLOW.md`: task 진행 절차
- `docs/TESTING.md`: 검증 방식
- `docs/PLANS.md`: 진행 상태 표

## 제작 기준
- 플랫폼: Windows PC
- 기술 방향: C++ + Win32 API + OpenGL(opengl32) + GDI 폰트 + XAudio2
- 목표 용량: 1.44MB 제약을 의식한 경량 프로토타입
- 외부 라이브러리: 기본적으로 사용 금지. GLFW, SDL, SFML, GLUT, GLEW, GLAD, ImGui, Assimp, stb 계열도 1차 프로토타입에서는 참조하지 않는다.
- 첫 목표: 3개 STL 버그 모듈을 진단하고 패치하는 플레이 가능한 짧은 데모
