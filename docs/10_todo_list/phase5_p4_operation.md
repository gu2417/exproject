# Phase 5 — P4 Operation Info

> **전제**: Phase 4 (P3 Convenience) 완료 후 시작.  
> 이 Phase가 끝나면 테스트 데이터 기반 도착 예정시간, 혼잡도, 운행 상태 표시가 완성된다.

---

### [P4-01] 도착 예정시간 조회 서버 핸들러

- **구현 파일**:
  - `src/server/operation_service.c/h` 또는 운행 정보 담당 파일
  - `src/server/router.c/h`
- **참조 문서**:
  - `docs/02_features/operation_info.md` — FR-O02
  - `docs/08_api/packets/operation.md`
  - `docs/07_database/tables/arrival_test.md`
  - `docs/07_database/query_catalog.md` — Q-O01
- **수용 기준**:
  - `ARRIVAL_REQ|<node_id>:<route_no>:<time_slot>` 처리
  - 노선 번호, 노드명, 도착 예정 분 반환
  - 테스트 데이터가 없으면 `ARRIVAL_RES|1|` 반환

---

### [P4-02] 운행 상태 표시

- **구현 파일**:
  - `src/server/operation_service.c/h` 또는 운행 정보 담당 파일
  - `src/client/result_viewer.c/h` 또는 결과 출력 담당 파일
- **참조 문서**:
  - `docs/02_features/operation_info.md` — FR-O01
  - `docs/06_ui_ux/components.md`
  - `docs/07_database/tables/arrival_test.md`
- **수용 기준**:
  - `운행 시작`, `곧 도착`, `운행 종료` 중 하나를 표시
  - 도착 예정시간 응답에 상태 포함
  - 알 수 없는 상태값은 안전한 기본 안내로 처리

---

### [P4-03] 혼잡도 조회 서버 핸들러

- **구현 파일**:
  - `src/server/operation_service.c/h` 또는 운행 정보 담당 파일
  - `src/server/router.c/h`
- **참조 문서**:
  - `docs/02_features/operation_info.md` — FR-O03
  - `docs/08_api/packets/operation.md`
  - `docs/07_database/tables/crowding_test.md`
- **수용 기준**:
  - `CROWDING_REQ|<node_id>:<route_no>:<time_slot>` 처리
  - `여유`, `보통`, `혼잡` 등급 반환
  - 테스트 데이터가 없으면 `CROWDING_RES|1|` 반환

---

### [P4-04] 시간대 기반 계산 및 테스트 입력

- **구현 파일**:
  - `src/server/operation_service.c/h` 또는 운행 정보 담당 파일
  - `src/client/menu.c/h`
- **참조 문서**:
  - `docs/02_features/operation_info.md` — FR-O04
  - `docs/07_database/tables/arrival_test.md`
  - `docs/07_database/tables/crowding_test.md`
- **수용 기준**:
  - 현재 시간 또는 사용자 입력 시간대를 기준으로 조회
  - 시연용 시간대 입력을 허용
  - 시간대 값이 비어 있거나 잘못되면 안내 후 재입력

---

### [P4-05] 검색 결과에 운행 정보 통합 표시

- **구현 파일**:
  - `src/client/result_viewer.c/h` 또는 결과 출력 담당 파일
  - `src/client/menu.c/h`
- **참조 문서**:
  - `docs/06_ui_ux/screens/results.md`
  - `docs/02_features/operation_info.md`
- **수용 기준**:
  - 정류장 검색 결과에서 도착 예정시간과 운행 상태를 함께 표시 가능
  - 혼잡도 조회 결과를 사용자에게 명확히 표시
  - 운행 정보가 없을 때 검색 결과 자체는 유지하고 별도 안내 출력

