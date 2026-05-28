# 구현 TODO 리스트

## 개요

이 폴더는 `docs/01~09`의 설계 문서를 기반으로 작성된 단계별 구현 TODO 목록을 담고 있다.  
각 항목에는 구현 파일 경로, 참조 문서, 완료 기준이 명시되어 있다.

## 소스코드 경로 규칙

```text
프로젝트 루트/
├── Makefile
├── src/
│   ├── server/          ← 서버 소스
│   ├── client/          ← 클라이언트 소스
│   └── common/          ← 공통 소스
├── data/                ← 공공데이터 CSV + 테스트 CSV
├── client_data/         ← 즐겨찾기·최근 검색 기록
├── logs/                ← 서버 로그
└── docs/
```

## 파일 구성

| 파일 | 내용 | 우선순위 |
|------|------|----------|
| [phase0_foundation.md](./phase0_foundation.md) | 빌드·공통 레이어·프로토콜·기본 골격 | **필수 선행** |
| [phase1_p0_core.md](./phase1_p0_core.md) | 서버/클라이언트 연결, CSV 로드, 연결리스트, 그래프 | P0 |
| [phase2_p1_search.md](./phase2_p1_search.md) | 정류장·버스·지하철역 핵심 검색 | P1 |
| [phase3_p2_expansion.md](./phase3_p2_expansion.md) | 환승 정보, 편의시설, 테스트 데이터 | P2 |
| [phase4_p3_convenience.md](./phase4_p3_convenience.md) | 즐겨찾기, 최근 검색, 메뉴 UI, 서버 로그 | P3 |
| [phase5_p4_operation.md](./phase5_p4_operation.md) | 도착 예정시간, 혼잡도, 운행 상태 표시 | P4 |
| [cross_cutting.md](./cross_cutting.md) | 입력 검증·예외 처리·로깅·한글 처리 | 상시 |

## 우선순위 기준

- **P0**: 프로그램 실행 기반 (서버/클라이언트 + 데이터 로딩 + 자료구조)
- **P1**: 핵심 검색 기능 (정류장, 버스 번호, 지하철역)
- **P2**: 확장 조회 기능 (환승, 편의시설, 테스트 데이터)
- **P3**: 사용자 편의 및 운영 확인성 (즐겨찾기, 최근 검색, 로그)
- **P4**: 운행 정보 표시 완성 (도착 예정시간, 혼잡도, 운행 상태)

## 의존성 순서

```text
Phase 0 -> Phase 1 (P0) -> Phase 2 (P1) -> Phase 3 (P2) -> Phase 4 (P3) -> Phase 5 (P4)
                                                                                 ^
                                                     Cross-Cutting (전 단계 병행)
```

## 구현 전 확인

| 항목 | 확인 내용 |
|------|-----------|
| 요구사항 범위 | 채팅, 학생정보관리, GUI, 실시간 API는 제외 |
| 데이터 파일 | `data/` 폴더의 CSV 목록과 컬럼을 문서와 일치 |
| 프로토콜 | `docs/08_api/packet_format.md`와 패킷 문서 기준 |
| 메뉴 | `docs/06_ui_ux/commands.md`의 1~8, 0번 구성 유지 |

