# Phase 2 — P1 Search

> **전제**: Phase 1 (P0 Core) 완료 후 시작.  
> 이 Phase가 끝나면 정류장, 버스 번호, 지하철역 검색이 동작하는 핵심 조회 MVP가 완성된다.

---

### [P1-01] 정류장 검색 서버 핸들러

- **구현 파일**:
  - `src/server/search_service.c/h`
  - `src/server/router.c/h`
- **참조 문서**:
  - `docs/02_features/transport_search.md` — FR-S01
  - `docs/08_api/packets/search.md`
  - `docs/07_database/query_catalog.md` — Q-S01
- **수용 기준**:
  - `STOP_SEARCH_REQ|<keyword>` 처리
  - 정류장명 부분 검색
  - 정류장별 경유 버스 목록 포함
  - 결과 없음 시 `STOP_SEARCH_RES|1|` 반환

---

### [P1-02] 버스 번호 검색 서버 핸들러

- **구현 파일**:
  - `src/server/search_service.c/h`
  - `src/server/router.c/h`
- **참조 문서**:
  - `docs/02_features/transport_search.md` — FR-S02
  - `docs/08_api/packets/search.md`
  - `docs/07_database/query_catalog.md` — Q-S02
- **수용 기준**:
  - `BUS_SEARCH_REQ|<route_no>` 처리
  - 출발지, 종점, 경유 정류장 순서 출력
  - 노선별 정류장 순서를 `seq` 기준으로 표시
  - 결과 없음 시 `BUS_SEARCH_RES|1|` 반환

---

### [P1-03] 지하철역 검색 서버 핸들러

- **구현 파일**:
  - `src/server/search_service.c/h`
  - `src/server/router.c/h`
- **참조 문서**:
  - `docs/02_features/transport_search.md` — FR-S03
  - `docs/08_api/packets/search.md`
  - `docs/07_database/query_catalog.md` — Q-S03
- **수용 기준**:
  - `SUBWAY_SEARCH_REQ|<keyword>` 처리
  - 역명 부분 검색
  - 노선 및 환승 가능 노선 표시
  - 결과 없음 시 `SUBWAY_SEARCH_RES|1|` 반환

---

### [P1-04] 검색 결과 없음 처리 통합

- **구현 파일**:
  - `src/server/search_service.c/h`
  - `src/client/result_viewer.c/h` 또는 결과 출력 담당 파일
- **참조 문서**:
  - `docs/02_features/transport_search.md` — FR-S06
  - `docs/08_api/error_codes.md`
  - `docs/06_ui_ux/screens/error.md`
- **수용 기준**:
  - 결과 없음과 서버 오류를 구분
  - 클라이언트에서 "검색 결과가 없습니다." 표시
  - 빈 응답으로 프로그램이 멈추지 않음

---

### [P1-05] 클라이언트 검색 메뉴 연결

- **구현 파일**:
  - `src/client/menu.c/h`
  - `src/client/request_builder.c/h` 또는 요청 생성 담당 파일
  - `src/client/result_viewer.c/h` 또는 결과 출력 담당 파일
- **참조 문서**:
  - `docs/06_ui_ux/commands.md`
  - `docs/06_ui_ux/screens/search.md`
  - `docs/06_ui_ux/screens/results.md`
- **수용 기준**:
  - 메뉴 1번: 정류장 검색 요청 전송
  - 메뉴 2번: 버스 번호 검색 요청 전송
  - 메뉴 3번: 지하철역 검색 요청 전송
  - 서버 응답을 사용자가 읽기 쉬운 목록으로 출력

