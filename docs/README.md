# Asan Public Transit Information System - 설계 문서

**버전**: 1.0.0-design.1  
**기준**: [`../requirement.md`](../requirement.md) v1.0.0  
**언어**: 한국어 (기술 용어만 영어)  
**다이어그램**: [Mermaid](https://mermaid.js.org/)

## 문서 구성

| # | 섹션 | 목적 |
|---|------|------|
| 01 | [overview](./01_overview/) | 프로젝트 목적, 범위, 플랫폼, NFR, 용어 |
| 02 | [features](./02_features/) | 기능 요구사항(FR) 상세 |
| 03 | [architecture](./03_architecture/) | 서버/클라이언트 구성, 세션, 데이터 흐름 |
| 04 | [file_structure](./04_file_structure/) | 소스 트리, 모듈 책임, 네이밍 |
| 05 | [security](./05_security/) | 입력 검증, 파일 안전성, 위협 모델 |
| 06 | [ui_ux](./06_ui_ux/) | 콘솔 화면, 명령 흐름, 출력 규칙 |
| 07 | [database](./07_database/) | CSV/텍스트 파일 스키마, 검색 카탈로그 |
| 08 | [api](./08_api/) | TCP 텍스트 패킷 프로토콜 명세 |
| 09 | [exception](./09_exception/) | 예외 분류, 처리, 로그 진단 |
| 10 | [todo_list](./10_todo_list/) | 단계별 구현 TODO, 수용 기준, 상시 적용 항목 |

## 읽는 순서

**처음 읽는 사람**: `01 -> 02 -> 03 -> 06 -> 07 -> 08 -> 05 -> 09 -> 04 -> 10`  
**구현자**: `10 -> 04 -> 08 -> 07 -> 03 -> 09` 순서로 참조  
**리뷰어**: `01 -> 10 -> 02 -> 05 -> 09` 순서로 참조

## 문서 규약

- 파일/함수/매크로/패킷 타입은 `backtick` 으로 표기한다.
- 모든 FR/NFR ID는 `requirement.md`의 ID를 그대로 사용한다.
- 다이어그램은 mermaid 코드블록으로만 작성한다.
- 실시간 API 기반 기능은 현재 범위에서 제외하고, CSV 및 테스트 데이터 기반으로 설명한다.
- 구현 작업은 `10_todo_list`의 Phase 순서를 기준으로 진행한다.
