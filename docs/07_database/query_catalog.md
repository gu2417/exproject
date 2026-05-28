# 조회 카탈로그

## Q-S01 정류장 검색

| 항목 | 내용 |
|------|------|
| 입력 | `keyword` |
| 조회 | `StopNode` 연결리스트에서 `stop_name` 부분 검색 |
| 후처리 | `route_stops`를 통해 경유 `route_no` 목록 수집 |
| 응답 | `STOP_SEARCH_RES|code|stop_id:stop_name:routes;...` |

## Q-S02 버스 번호 검색

| 항목 | 내용 |
|------|------|
| 입력 | `route_no` |
| 조회 | `RouteNode` 검색 후 `RouteStopNode` seq 순회 |
| 후처리 | stop_id를 stop_name으로 변환 |
| 응답 | `BUS_SEARCH_RES|code|route_no:start:end:stop1,stop2,...` |

## Q-S03 지하철역 검색

| 항목 | 내용 |
|------|------|
| 입력 | `keyword` |
| 조회 | `StationNode` 연결리스트에서 역명 부분 검색 |
| 응답 | `SUBWAY_SEARCH_RES|code|station_id:station_name:line_no:transfer_lines;...` |

## Q-S04 길찾기 최단 경로

| 항목 | 내용 |
|------|------|
| 입력 | `from_node_keyword`, `to_node_keyword` |
| 조회 | 키워드를 노드 ID로 해석 후 그래프 가중치 기반 최단 경로 탐색 |
| 응답 | `TRANSFER_RES|code|path:transfer_count:estimated_minutes` |

## Q-S05 편의시설 조회

| 항목 | 내용 |
|------|------|
| 입력 | `node_keyword`, `category` |
| 조회 | 노드 ID 해석 후 `FacilityNode` 연결리스트 필터링 |
| 응답 | `FACILITY_RES|code|name:category:nearest_node:address;...` |

## Q-O01 운행 정보

| 항목 | 내용 |
|------|------|
| 입력 | `node_id`, `route_no`, `time_slot` |
| 조회 | `arrival_test`, `crowding_test` 캐시 |
| 응답 | `ARRIVAL_RES`, `CROWDING_RES` |
