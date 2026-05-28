# Phase 0 — Foundation

> **선행 필수**: 이 Phase가 완료되어야 모든 후속 Phase를 시작할 수 있다.

---

### [F0-01] 프로젝트 디렉터리와 Makefile 작성

- **구현 파일**: `Makefile`, `src/`, `data/`, `client_data/`, `logs/`
- **참조 문서**:
  - `docs/03_architecture/build_and_portability.md`
  - `docs/04_file_structure/repo_layout.md`
- **수용 기준**:
  - `make` 실행 시 서버·클라이언트 바이너리 생성
  - `make clean` 동작
  - `src/server`, `src/client`, `src/common` 구조 생성
  - `data`, `client_data`, `logs` 폴더 경로를 코드에서 참조 가능

---

### [F0-02] 공통 프로토콜 상수 정의

- **구현 파일**: `src/common/protocol.h`
- **참조 문서**:
  - `docs/08_api/packet_format.md`
  - `docs/08_api/error_codes.md`
  - `docs/08_api/packets/search.md`
  - `docs/08_api/packets/transfer.md`
  - `docs/08_api/packets/facility.md`
  - `docs/08_api/packets/operation.md`
- **수용 기준**:
  - `MAX_PACKET_SIZE 4096` 정의
  - 구분자 상수: `FIELD_SEP '|'`, `ITEM_SEP ':'`, `LIST_SEP ';'`, `PACKET_TERM '\n'`
  - `STOP_SEARCH_REQ`, `BUS_SEARCH_REQ`, `SUBWAY_SEARCH_REQ`, `TRANSFER_REQ`, `FACILITY_REQ`, `ARRIVAL_REQ`, `CROWDING_REQ` 정의
  - 오류 코드 `OK`, `NOT_FOUND`, `INVALID_REQUEST`, `DATA_LOAD_ERROR`, `SERVER_ERROR` 정의

---

### [F0-03] 공통 자료형 정의

- **구현 파일**: `src/common/types.h`
- **참조 문서**:
  - `docs/01_overview/glossary.md`
  - `docs/07_database/er_diagram.md`
  - `docs/07_database/tables/bus_stops.md`
  - `docs/07_database/tables/bus_routes.md`
  - `docs/07_database/tables/facilities.md`
- **수용 기준**:
  - `StopNode`, `RouteNode`, `RouteStopNode`, `FacilityNode` 구조체 정의
  - 지하철역 저장용 연결리스트 구조체 정의
  - `GraphNode`, `GraphEdge`, `TransferGraph` 정의
  - `AppData`, `ClientSession`, `ServerContext` 정의
  - 문자열 배열 크기는 `requirement.md`의 구조체 기준과 충돌하지 않음

---

### [F0-04] 문자열·시간·파일 유틸리티 작성

- **구현 파일**:
  - `src/common/string_utils.c`
  - `src/common/string_utils.h`
  - `src/common/file_utils.c`
  - `src/common/file_utils.h`
- **참조 문서**:
  - `docs/05_security/input_validation.md`
  - `docs/05_security/injection_prevention.md`
  - `docs/09_exception/edge_cases.md`
- **수용 기준**:
  - `trim`, 안전한 문자열 복사, 빈 문자열 검사 함수 제공
  - 패킷 구분자 포함 여부 검사
  - 로그용 timestamp 문자열 생성
  - `data/`, `client_data/`, `logs/` 폴더 존재 확인 또는 생성

---

### [F0-05] CSV 파서 기본 구현

- **구현 파일**:
  - `src/common/csv_parser.c`
  - `src/common/csv_parser.h`
- **참조 문서**:
  - `docs/07_database/README.md`
  - `docs/07_database/tables/bus_stops.md`
  - `docs/09_exception/db_failure_modes.md`
- **수용 기준**:
  - 한 줄 CSV를 필드 배열로 분리
  - 헤더 컬럼 존재 여부 확인
  - 필드 개수 부족 시 오류 반환
  - UTF-8 텍스트를 손상시키지 않고 처리

---

### [F0-06] 네트워크 호환 계층 작성

- **구현 파일**:
  - `src/common/net_compat.h`
  - 플랫폼별 소켓 초기화/종료 구현 파일
- **참조 문서**:
  - `docs/01_overview/target_platforms.md`
  - `docs/03_architecture/build_and_portability.md`
- **수용 기준**:
  - 서버·클라이언트가 같은 함수명으로 socket close 처리 가능
  - Windows/Linux 소켓 차이를 공통 함수로 감춤
  - 소켓 오류 메시지를 로그 또는 콘솔에 출력 가능

---

### [F0-07] 기본 서버·클라이언트 골격 작성

- **구현 파일**:
  - `src/server/main.c`
  - `src/server/server.c/h`
  - `src/server/client_handler.c/h`
  - `src/client/main.c`
  - `src/client/client_net.c/h`
- **참조 문서**:
  - `docs/03_architecture/system_context.md`
  - `docs/03_architecture/session_lifecycle.md`
  - `docs/09_exception/server_error_handling.md`
  - `docs/09_exception/client_error_handling.md`
- **수용 기준**:
  - 서버가 지정 포트에서 접속 대기
  - 클라이언트가 서버 IP/포트로 접속
  - 접속 성공/실패 메시지 출력
  - 종료 시 소켓 정리

