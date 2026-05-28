# bus_routes.csv

버스 노선 기본 정보다.

| 컬럼 | 타입 | 필수 | 설명 |
|------|------|------|------|
| `route_no` | string | Y | 버스 번호 |
| `route_name` | string | N | 노선명 |
| `start_stop_id` | string | Y | 출발 정류장 ID |
| `end_stop_id` | string | Y | 종점 정류장 ID |
| `interval_min` | int | N | 배차 간격 |

## 검증

- `route_no`는 비어 있으면 안 된다.
- `start_stop_id`, `end_stop_id`는 `bus_stops.csv`에 존재해야 한다.
- `interval_min`이 숫자가 아니면 기본값 또는 알 수 없음으로 처리한다.

