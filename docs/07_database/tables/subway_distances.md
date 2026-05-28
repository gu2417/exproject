# subway_distances.csv

지하철 인접역 간 거리와 예상 이동시간 정보다.
천안·아산권 수도권 전철 1호선 역간 경로를 환승 그래프 간선으로 만들 때 사용한다.

## 컬럼

| 컬럼 | 타입 | 필수 | 설명 |
|------|------|------|------|
| `from_station_id` | string | Y | 출발 지하철역 ID |
| `to_station_id` | string | Y | 도착 지하철역 ID |
| `line_no` | string | N | 노선명 |
| `distance_km` | number | Y | 인접역 간 거리(km) |
| `estimated_minutes` | number | N | 예상 이동시간(분) |

## 관계

- `from_station_id`, `to_station_id`는 `subway_stations.station_id`를 참조한다.
- 서버는 양방향 간선을 생성하므로 CSV에는 인접역 구간을 한 번만 기록한다.
- `estimated_minutes`가 비어 있으면 서버가 거리 기준 기본값으로 보정할 수 있다.
