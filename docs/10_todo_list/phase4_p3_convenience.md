# Phase 4 — P3 Convenience

> **전제**: Phase 3 (P2 Expansion) 완료 후 시작.  
> 이 Phase가 끝나면 즐겨찾기, 최근 검색 기록, 최종 메뉴 UI, 서버 로그가 완성된다.

---

### [P3-01] 즐겨찾기 추가

- **구현 파일**:
  - `src/client/favorite.c/h`
  - `src/client/menu.c/h`
- **참조 문서**:
  - `docs/02_features/user_convenience.md` — FR-U01
  - `docs/07_database/tables/favorites.md`
  - `docs/06_ui_ux/screens/favorites.md`
- **수용 기준**:
  - 정류장, 버스 번호, 지하철역을 즐겨찾기에 추가
  - `client_data/favorites.txt`에 저장
  - 저장 성공 메시지 출력
  - 파일이 없으면 생성

---

### [P3-02] 즐겨찾기 조회 및 중복 방지

- **구현 파일**:
  - `src/client/favorite.c/h`
- **참조 문서**:
  - `docs/02_features/user_convenience.md` — FR-U02, FR-U03
  - `docs/07_database/tables/favorites.md`
- **수용 기준**:
  - 메뉴 6번에서 즐겨찾기 목록 출력
  - 동일 유형과 동일 ID는 중복 저장하지 않음
  - 빈 목록이면 안내 메시지 출력
  - 손상된 행은 건너뛰고 계속 표시

---

### [P3-03] 즐겨찾기 삭제

- **구현 파일**:
  - `src/client/favorite.c/h`
  - `src/client/menu.c/h`
- **참조 문서**:
  - `docs/02_features/user_convenience.md` — FR-U04
  - `docs/06_ui_ux/screens/favorites.md`
- **수용 기준**:
  - 메뉴 8번에서 즐겨찾기 목록을 확인하고 번호로 삭제
  - 삭제 후 `client_data/favorites.txt`를 재저장
  - 없는 번호 또는 빈 목록이면 안내 메시지 출력

---

### [P3-04] 최근 검색 기록 저장

- **구현 파일**:
  - `src/client/recent_search.c/h`
  - `src/client/menu.c/h`
- **참조 문서**:
  - `docs/02_features/user_convenience.md` — FR-U05
  - `docs/07_database/tables/recent_search.md`
- **수용 기준**:
  - 검색 유형, 키워드, 시간을 `client_data/recent_search.txt`에 저장
  - 정류장, 버스, 지하철역, 환승, 편의시설 조회 입력을 기록
  - 파일 열기 실패 시 안내 후 검색 기능은 계속 사용 가능

---

### [P3-05] 최근 검색 기록 조회

- **구현 파일**:
  - `src/client/recent_search.c/h`
  - `src/client/menu.c/h`
- **참조 문서**:
  - `docs/02_features/user_convenience.md` — FR-U06
  - `docs/06_ui_ux/screens/history.md`
- **수용 기준**:
  - 메뉴 9번에서 최근 검색 기록 출력
  - 검색 유형, 키워드, 시간 표시
  - 기록이 없으면 빈 목록 안내
  - 파일 손상 시 읽을 수 있는 행만 표시

---

### [P3-06] 최종 메뉴 UI 정리

- **구현 파일**:
  - `src/client/menu.c/h`
  - `src/client/main.c`
- **참조 문서**:
  - `docs/02_features/user_convenience.md` — FR-U07
  - `docs/06_ui_ux/commands.md`
  - `docs/06_ui_ux/screens/main_menu.md`
- **수용 기준**:
  - 1~9번, 0번 종료 메뉴가 문서와 일치
  - 잘못된 메뉴 번호 입력 시 안내 후 복귀
  - 각 기능 완료 후 메인 메뉴로 복귀

---

### [P3-07] 서버 로그 저장

- **구현 파일**:
  - `src/server/log.c/h`
  - `src/server/client_handler.c/h`
  - `src/server/router.c/h`
- **참조 문서**:
  - `docs/02_features/server_client_logging.md` — FR-N04
  - `docs/09_exception/logging_and_diagnostics.md`
- **수용 기준**:
  - `logs/server.log`에 접속, 요청 타입, 처리 결과, 오류 기록
  - 서버 콘솔에도 주요 로그 출력
  - 여러 요청이 동시에 들어와도 로그 라인이 섞이지 않음
