# facilities.csv

정류장 또는 지하철역 주변 편의시설 정보다.

| 컬럼 | 타입 | 필수 | 설명 |
|------|------|------|------|
| `facility_id` | string | Y | 시설 ID |
| `name` | string | Y | 시설명 |
| `category` | string | Y | 시설 카테고리 |
| `nearest_node_id` | string | Y | 가장 가까운 정류장 또는 역 ID |
| `address` | string | N | 주소 |
| `lat` | double | N | 위도 |
| `lng` | double | N | 경도 |

## 검증

- `nearest_node_id`는 정류장 ID 또는 역 ID 중 하나와 매칭되어야 한다.
- 카테고리가 비어 있으면 `기타`로 대체할 수 있다.

## 현재 카테고리

- 병원
- 약국
- 편의점
- 카페
- PC방
- 음식점
