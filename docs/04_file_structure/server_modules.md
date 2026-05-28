# 서버 모듈

| 파일 | 책임 |
|------|------|
| `main.c` | 서버 설정, 데이터 로딩, 서버 시작 |
| `server.c/h` | listen socket 생성, accept loop |
| `client_handler.c/h` | 클라이언트별 패킷 수신/응답 송신 |
| `router.c/h` | 패킷 타입별 핸들러 호출 |
| `search_service.c/h` | 정류장, 버스, 지하철역 검색 |
| `transfer_service.c/h` | 길찾기 최단 경로 탐색 |
| `facility_service.c/h` | 주변 편의시설 조회 |
| `operation_service.c/h` | 도착 예정시간, 운행 상태, 혼잡도 조회 |
| `data_loader.c/h` | CSV 파일 로딩 및 `AppData` 구성 |
| `log.c/h` | 서버 로그 출력 및 파일 저장 |

## 서버 모듈 규칙

- 서비스 함수는 요청 필드를 검증한 뒤 공통 응답 형식으로 반환한다.
- 로딩된 데이터는 가능한 읽기 전용으로 다룬다.
- 파일 열기 실패, 파싱 실패, 검색 결과 없음은 명확한 오류 코드로 변환한다.
