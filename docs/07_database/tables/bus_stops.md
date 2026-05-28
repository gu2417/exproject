# bus_stops.csv

아산시 버스 정류장 목록이다.

| 컬럼 | 타입 | 필수 | 설명 |
|------|------|------|------|
| `stop_id` | string | Y | 정류장 고유 ID |
| `stop_name` | string | Y | 정류장명 |
| `lat` | double | N | 위도 |
| `lng` | double | N | 경도 |
| `district` | string | N | 행정구역 또는 지역명 |

## 검증

- `stop_id`는 비어 있으면 안 된다.
- `stop_id` 중복은 오류로 기록한다.
- `lat`, `lng`가 숫자가 아니면 위치 정보 없음으로 처리하거나 해당 행을 제외한다.

