# route_stops.csv

노선별 경유 정류장 순서다.

| 컬럼 | 타입 | 필수 | 설명 |
|------|------|------|------|
| `route_no` | string | Y | 버스 번호 |
| `seq` | int | Y | 정류장 순서 |
| `stop_id` | string | Y | 정류장 ID |
| `direction` | string | N | 운행 방향 |

## 검증

- `route_no`는 `bus_routes.csv`에 존재해야 한다.
- `stop_id`는 `bus_stops.csv`에 존재해야 한다.
- 같은 `route_no` 안에서 `seq`가 중복되면 로그에 기록한다.

