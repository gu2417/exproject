# 서버 구성 요소

## 1. 컴포넌트 목록

| 컴포넌트 | 주요 파일 | 책임 |
|----------|-----------|------|
| Server Main | `src/server/main.c` | 설정 로드, CSV 로딩, listen socket 생성 |
| Server Core | `src/server/server.c/h` | bind/listen/accept 루프 관리 |
| Client Handler | `src/server/client_handler.c/h` | 클라이언트별 recv/send 루프 |
| Router | `src/server/router.c/h` | 패킷 타입별 핸들러 분기 |
| Search Service | `src/server/search_service.c/h` | 정류장, 버스, 지하철역 검색 |
| Transfer Service | `src/server/transfer_service.c/h` | 그래프 기반 길찾기 최단 경로 탐색 |
| Facility Service | `src/server/facility_service.c/h` | 주변 편의시설 조회 |
| Operation Service | `src/server/operation_service.c/h` | 도착 예정시간, 운행 상태, 혼잡도 조회 |
| Data Loader | `src/server/data_loader.c/h` | CSV 파일 로딩 및 검증 |
| Logger | `src/server/log.c/h` | 서버 로그 출력 및 파일 저장 |

## 2. 요청 처리 순서

1. `server.c`가 클라이언트 연결을 accept 한다.
2. `client_handler.c`가 한 줄 단위 패킷을 수신한다.
3. `router.c`가 `TYPE`을 기준으로 서비스 함수를 호출한다.
4. 서비스 함수는 `AppData`의 연결리스트 또는 그래프를 조회한다.
5. 응답 문자열을 만들어 클라이언트에 전송한다.
6. 처리 결과와 오류를 `logs/server.log`에 기록한다.

## 3. 서버 공유 데이터

| 데이터 | 접근 방식 |
|--------|-----------|
| `AppData` | 서버 시작 시 로드 후 읽기 중심 사용 |
| `TransferGraph` | 환승 요청에서 읽기 중심 사용 |
| `ClientSession[]` | 접속/종료 시 갱신, mutex 보호 권장 |
| 로그 파일 | 단일 로그 함수 또는 mutex로 쓰기 보호 |
