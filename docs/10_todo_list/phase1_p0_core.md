# Phase 1 — P0 Core

> **전제**: Phase 0 (Foundation) 완료 후 시작.  
> 이 Phase가 끝나면 서버/클라이언트 연결, CSV 로딩, 연결리스트 저장, 환승 그래프 기본 생성이 동작한다.

---

### [P0-01] 서버 실행 및 접속 대기

- **구현 파일**:
  - `src/server/main.c`
  - `src/server/server.c/h`
- **참조 문서**:
  - `docs/02_features/server_client_logging.md` — FR-N01
  - `docs/03_architecture/server_components.md`
  - `docs/09_exception/server_error_handling.md`
- **수용 기준**:
  - 지정 포트에서 listen 시작
  - 포트 사용 중 또는 socket 생성 실패 시 오류 출력
  - 클라이언트 접속 시 IP와 접속 시간을 로그에 기록

---

### [P0-02] 클라이언트 접속 구현

- **구현 파일**:
  - `src/client/main.c`
  - `src/client/client_net.c/h`
- **참조 문서**:
  - `docs/02_features/server_client_logging.md` — FR-N02
  - `docs/03_architecture/client_components.md`
  - `docs/09_exception/client_error_handling.md`
- **수용 기준**:
  - 서버 IP와 포트를 입력받아 TCP 연결
  - 연결 실패 시 안내 메시지 출력
  - 연결 성공 후 메인 메뉴로 진입 가능

---

### [P0-03] 패킷 수신 루프와 요청 라우터 구현

- **구현 파일**:
  - `src/server/client_handler.c/h`
  - `src/server/router.c/h`
- **참조 문서**:
  - `docs/02_features/server_client_logging.md` — FR-N03
  - `docs/08_api/packet_format.md`
  - `docs/08_api/error_codes.md`
- **수용 기준**:
  - `\n` 기준으로 한 패킷씩 분리
  - 요청 타입에 따라 서비스 함수로 분기
  - 알 수 없는 요청은 `ERROR|2:INVALID_REQUEST` 응답
  - 패킷 처리 결과를 로그에 기록

---

### [P0-04] CSV 데이터 로더 구현

- **구현 파일**:
  - `src/server/data_loader.c/h`
  - `src/common/csv_parser.c/h`
- **참조 문서**:
  - `docs/02_features/data_loading.md` — FR-DATA01, FR-DATA04
  - `docs/07_database/tables/bus_stops.md`
  - `docs/07_database/tables/bus_routes.md`
  - `docs/07_database/tables/route_stops.md`
  - `docs/07_database/tables/subway_stations.md`
  - `docs/07_database/tables/facilities.md`
- **수용 기준**:
  - 서버 시작 시 필수 CSV 파일 로드
  - 필수 컬럼 누락, 빈 ID, 중복 ID, 잘못된 참조 검사
  - 오류 행은 로그에 기록
  - 치명적 데이터 오류는 `DATA_LOAD_ERROR`로 처리

---

### [P0-05] 연결리스트 저장 구현

- **구현 파일**:
  - `src/common/linked_list.c`
  - `src/common/linked_list.h`
  - `src/common/types.h`
- **참조 문서**:
  - `docs/02_features/data_loading.md` — FR-DATA02
  - `docs/03_architecture/data_flow.md`
  - `docs/07_database/er_diagram.md`
- **수용 기준**:
  - 정류장, 노선, 노선별 정류장, 지하철역, 편의시설을 연결리스트로 저장
  - 삽입, 검색, 순회, 해제 함수 구현
  - 서버 종료 시 동적 할당 메모리 해제

---

### [P0-06] 환승 그래프 기본 생성

- **구현 파일**:
  - `src/common/graph.c`
  - `src/common/graph.h`
  - `src/server/data_loader.c/h`
- **참조 문서**:
  - `docs/02_features/data_loading.md` — FR-DATA03
  - `docs/03_architecture/data_flow.md`
  - `docs/07_database/er_diagram.md`
- **수용 기준**:
  - 정류장과 지하철역을 그래프 노드로 등록
  - 이동 가능 구간을 간선으로 등록
  - 노드 ID로 그래프 index를 찾을 수 있음
  - 그래프 생성 실패 또는 참조 오류를 로그에 기록

---

### [P0-07] 연결 종료 처리

- **구현 파일**:
  - `src/server/client_handler.c/h`
  - `src/client/client_net.c/h`
- **참조 문서**:
  - `docs/02_features/server_client_logging.md` — FR-N05
  - `docs/03_architecture/session_lifecycle.md`
  - `docs/09_exception/edge_cases.md`
- **수용 기준**:
  - 클라이언트 종료 시 서버 세션 정리
  - 네트워크 오류 시 소켓 close
  - 종료 로그 기록
  - 비정상 종료가 서버 전체 크래시로 이어지지 않음

