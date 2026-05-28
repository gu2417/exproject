# 데이터 로딩 및 자료구조(FR-DATA01 ~ DATA05)

## FR-DATA01 CSV 파일 로드 - P0

| 항목 | 내용 |
|------|------|
| 입력 | `data/*.csv` |
| 처리 모듈 | `data_loader.c`, `csv_parser.c` |
| 로딩 시점 | 서버 시작 직후, listen 시작 전 |
| 성공 기준 | 필수 CSV를 모두 읽고 `AppData`에 저장 |
| 실패 처리 | `DATA_LOAD_ERROR` 로그 기록 후 서버 시작 중단 또는 기능 제한 |

## FR-DATA02 연결리스트 저장 - P0

| 항목 | 내용 |
|------|------|
| 대상 | 정류장, 노선, 노선별 정류장, 지하철역, 지하철 역간거리, 편의시설 |
| 구조체 | `StopNode`, `RouteNode`, `RouteStopNode`, `StationNode`, `SubwayDistanceNode`, `FacilityNode` |
| 규칙 | 삽입, 검색, 순회, 해제 함수를 명시적으로 구현 |
| 성공 기준 | 검색 요청이 배열 원본이 아니라 연결리스트에서 처리됨 |

## FR-DATA03 그래프 생성 - P0

| 항목 | 내용 |
|------|------|
| 대상 | 정류장과 지하철역을 환승 그래프의 노드로 등록 |
| 간선 | 노선의 인접 정류장, 인접 지하철역, 환승 가능한 역/정류장 관계 |
| 가중치 | 역간거리 기반 예상 이동 분 또는 기본 테스트 가중치 |
| 사용 기능 | FR-S04 길찾기 최단 경로 |
| 성공 기준 | 출발지와 도착지 사이의 경로 노드 목록 출력 |

## FR-DATA04 데이터 검증 - P1

| 항목 | 내용 |
|------|------|
| 검증 대상 | 필수 컬럼, 빈 ID, 중복 ID, 잘못된 참조, 숫자 필드 |
| 처리 방식 | 오류 건수와 행 번호를 로그에 기록 |
| 허용 정책 | 치명적 파일 누락은 로딩 실패, 일부 행 오류는 건너뛰기 가능 |
| 관련 NFR | NFR-03, NFR-07, NFR-08 |

## FR-DATA05 테스트 데이터 추가 - P2

| 항목 | 내용 |
|------|------|
| 파일 | `arrival_test.csv`, `crowding_test.csv` |
| 목적 | 실시간 API 없이 도착 예정시간, 운행 상태, 혼잡도 시연 |
| 처리 방식 | 서버 시작 시 함께 로드하거나 운행 정보 조회 시 캐시에서 검색 |
| 관련 기능 | FR-O01 ~ FR-O04 |

## 관련 문서

- 데이터 스키마: [`07_database/`](../07_database/)
- 자료구조: [`03_architecture/data_flow.md`](../03_architecture/data_flow.md)
- 예외 처리: [`09_exception/server_error_handling.md`](../09_exception/server_error_handling.md)
