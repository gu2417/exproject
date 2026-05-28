# Cross-Cutting — 입력 검증·예외 처리·로깅

> 이 항목들은 특정 Phase에 속하지 않고 **전 단계에 걸쳐 지속적으로 적용**한다.  
> Phase 0 완료 직후부터 각 기능 구현과 동시에 반영해야 한다.

---

### [CC-01] 입력 검증

- **적용 위치**: 모든 패킷 핸들러, 모든 클라이언트 입력
- **구현 파일**:
  - `src/common/string_utils.c/h`
  - `src/server/router.c/h`
  - `src/client/menu.c/h`
- **참조 문서**:
  - `docs/05_security/input_validation.md`
  - `docs/08_api/packet_format.md`
  - `docs/09_exception/edge_cases.md`
- **수용 기준**:
  - 빈 검색어 차단
  - 패킷 최대 길이 4096 bytes 검사
  - 요청 타입별 필드 개수 검사
  - 구분자와 제어문자 입력 처리
  - 검증 실패 시 서버 크래시 없이 오류 응답 또는 사용자 안내

---

### [CC-02] 파일 안전성 및 CSV 검증

- **적용 위치**: CSV 로더, 즐겨찾기, 최근 검색, 서버 로그
- **구현 파일**:
  - `src/server/data_loader.c/h`
  - `src/client/favorite.c/h`
  - `src/client/recent_search.c/h`
  - `src/server/log.c/h`
- **참조 문서**:
  - `docs/05_security/injection_prevention.md`
  - `docs/07_database/README.md`
  - `docs/09_exception/db_failure_modes.md`
- **수용 기준**:
  - 사용자가 임의 파일 경로를 지정할 수 없음
  - CSV 필수 컬럼 누락 검사
  - 손상된 로컬 파일 행은 건너뜀
  - 로그에는 개행 제거 후 기록

---

### [CC-03] 예외 처리 통합

- **적용 위치**: 서버·클라이언트 전체
- **구현 파일**: 전체 `*.c/h`
- **참조 문서**:
  - `docs/09_exception/error_taxonomy.md`
  - `docs/09_exception/server_error_handling.md`
  - `docs/09_exception/client_error_handling.md`
  - `docs/08_api/error_codes.md`
- **수용 기준**:
  - 결과 없음, 요청 오류, 데이터 오류, 서버 오류를 구분
  - 클라이언트 연결 종료 시 서버 세션 정리
  - 파일 열기 실패 시 사용자에게 명확히 안내
  - 오류 발생 후 가능한 경우 메인 메뉴로 복귀

---

### [CC-04] 로그 및 진단

- **적용 위치**: 서버 시작, CSV 로딩, 요청 처리, 연결 종료
- **구현 파일**:
  - `src/server/log.c/h`
  - `src/server/main.c`
  - `src/server/client_handler.c/h`
  - `src/server/router.c/h`
- **참조 문서**:
  - `docs/09_exception/logging_and_diagnostics.md`
  - `docs/02_features/server_client_logging.md` — FR-N04
- **수용 기준**:
  - 서버 시작/종료 로그 기록
  - CSV 로딩 성공/실패 로그 기록
  - 클라이언트 IP, 요청 타입, 결과 코드 기록
  - 오류 메시지는 원인 추적이 가능할 정도로 구체적으로 기록

---

### [CC-05] 한글 및 UTF-8 처리

- **적용 위치**: CSV 파서, 콘솔 출력, 패킷 문자열
- **구현 파일**:
  - `src/common/csv_parser.c/h`
  - `src/common/string_utils.c/h`
  - `src/client/result_viewer.c/h` 또는 결과 출력 담당 파일
- **참조 문서**:
  - `docs/01_overview/non_functional.md` — NFR-09
  - `docs/06_ui_ux/design_tokens.md`
  - `docs/09_exception/db_failure_modes.md`
- **수용 기준**:
  - CSV 파일은 UTF-8 기준으로 읽음
  - 한글 검색어가 패킷에서 손상되지 않음
  - Windows 콘솔 한글 출력 문제 발생 시 안내 가능

