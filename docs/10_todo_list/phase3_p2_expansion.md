# Phase 3 — P2 Expansion

> **전제**: Phase 2 (P1 Search) 완료 후 시작.  
> 이 Phase가 끝나면 길찾기 최단 경로, 주변 편의시설 조회, 운행 테스트 데이터 로딩이 가능해진다.

---

### [P2-01] 환승 경로 탐색 구현

- **구현 파일**:
  - `src/server/transfer_service.c/h`
  - `src/common/graph.c/h`
  - `src/server/router.c/h`
- **참조 문서**:
  - `docs/02_features/transport_search.md` — FR-S04
  - `docs/08_api/packets/transfer.md`
  - `docs/07_database/query_catalog.md` — Q-S04
  - `docs/03_architecture/data_flow.md`
- **수용 기준**:
  - `TRANSFER_REQ|<from>:<to>` 처리
  - 출발지와 도착지를 그래프 노드로 해석
  - 경로 노드 목록, 환승 횟수, 예상 소요시간 반환
  - 경로 없음 시 `TRANSFER_RES|1|` 반환

---

### [P2-02] 주변 편의시설 조회 구현

- **구현 파일**:
  - `src/server/facility_service.c/h` 또는 `src/server/search_service.c/h`
  - `src/server/router.c/h`
- **참조 문서**:
  - `docs/02_features/transport_search.md` — FR-S05
  - `docs/08_api/packets/facility.md`
  - `docs/07_database/tables/facilities.md`
  - `docs/07_database/query_catalog.md` — Q-S05
- **수용 기준**:
  - `FACILITY_REQ|<node_keyword>:<category>` 처리
  - 정류장/역 ID와 카테고리 기준으로 편의시설 검색
  - 시설명, 종류, 가까운 노드, 주소 반환
  - 결과 없음 시 `FACILITY_RES|1|` 반환

---

### [P2-03] 테스트 데이터 CSV 로딩

- **구현 파일**:
  - `src/server/data_loader.c/h`
  - `src/common/types.h`
- **참조 문서**:
  - `docs/02_features/data_loading.md` — FR-DATA05
  - `docs/07_database/tables/arrival_test.md`
  - `docs/07_database/tables/crowding_test.md`
  - `docs/07_database/migration_and_seed.md`
- **수용 기준**:
  - `data/arrival_test.csv` 로드
  - `data/crowding_test.csv` 로드
  - 노선 번호, 노드 ID, 시간대 필드를 검증
  - 데이터 오류를 서버 로그에 기록

---

### [P2-04] 클라이언트 확장 조회 메뉴 연결

- **구현 파일**:
  - `src/client/menu.c/h`
  - `src/client/request_builder.c/h` 또는 요청 생성 담당 파일
  - `src/client/result_viewer.c/h` 또는 결과 출력 담당 파일
- **참조 문서**:
  - `docs/06_ui_ux/commands.md`
  - `docs/06_ui_ux/screens/search.md`
  - `docs/06_ui_ux/screens/results.md`
- **수용 기준**:
  - 메뉴 4번: 길찾기 최단 경로
  - 메뉴 5번: 주변 편의시설 조회
  - 입력값이 비어 있으면 서버 요청 전 오류 안내
  - 응답 결과를 목록 또는 경로 형태로 출력
