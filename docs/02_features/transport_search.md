# 교통 정보 검색(FR-S01 ~ S06)

## FR-S01 정류장 검색 - P1

| 항목 | 내용 |
|------|------|
| 입력 | 정류장명 또는 키워드 |
| 패킷 | `STOP_SEARCH_REQ` / `STOP_SEARCH_RES` |
| 데이터 | `bus_stops.csv`, `route_stops.csv`, `bus_routes.csv` |
| 출력 | 정류장 ID, 정류장명, 경유 버스 목록 |
| 예외 | 결과 없음 시 `NOT_FOUND` |

## FR-S02 버스 번호 검색 - P1

| 항목 | 내용 |
|------|------|
| 입력 | 버스 번호 |
| 패킷 | `BUS_SEARCH_REQ` / `BUS_SEARCH_RES` |
| 데이터 | `bus_routes.csv`, `route_stops.csv`, `bus_stops.csv` |
| 출력 | 노선 번호, 출발지, 종점, 경유 정류장 순서, 운행 방향 |
| 예외 | 없는 노선 번호는 `NOT_FOUND` |

## FR-S03 지하철역 검색 - P1

| 항목 | 내용 |
|------|------|
| 입력 | 지하철역명 또는 키워드 |
| 패킷 | `SUBWAY_SEARCH_REQ` / `SUBWAY_SEARCH_RES` |
| 데이터 | `subway_stations.csv` |
| 출력 | 역 ID, 역명, 노선 번호, 환승 가능 노선 |
| 예외 | 결과 없음 시 안내 메시지 출력 |

## FR-S04 길찾기 최단 경로 - P2

| 항목 | 내용 |
|------|------|
| 입력 | 탑승 위치, 도착 위치 |
| 패킷 | `TRANSFER_REQ` / `TRANSFER_RES` |
| 데이터 | `TransferGraph` |
| 출력 | 최단 경로, 환승 횟수, 예상 소요시간, 구간별 이동수단 |
| 탐색 | 가중치 기반 최단 경로 탐색 |

## FR-S05 주변 편의시설 조회 - P2

| 항목 | 내용 |
|------|------|
| 입력 | 정류장/역명, 카테고리 |
| 패킷 | `FACILITY_REQ` / `FACILITY_RES` |
| 데이터 | `facilities.csv` |
| 출력 | 편의시설명, 종류, 가까운 노드, 주소 |
| 카테고리 | 병원, 약국, 편의점, 카페, PC방, 음식점 |
| 예외 | 해당 위치 또는 카테고리에 결과가 없으면 `NOT_FOUND` |

## FR-S06 검색 결과 없음 처리 - P1

| 항목 | 내용 |
|------|------|
| 적용 대상 | 모든 검색성 요청 |
| 서버 응답 | `code=1` 또는 `ERROR|1:NOT_FOUND` |
| 클라이언트 표시 | "검색 결과가 없습니다." 형태의 명확한 메시지 |
| 성공 기준 | 빈 목록과 오류를 사용자가 구분할 수 있음 |

## 관련 문서

- 패킷: [`08_api/packets/search.md`](../08_api/packets/search.md)
- 데이터 파일: [`07_database/query_catalog.md`](../07_database/query_catalog.md)
- 콘솔 화면: [`06_ui_ux/screens/search.md`](../06_ui_ux/screens/search.md)
