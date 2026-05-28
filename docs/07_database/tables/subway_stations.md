# subway_stations.csv

지하철역 정보다.

| 컬럼 | 타입 | 필수 | 설명 |
|------|------|------|------|
| `station_id` | string | Y | 역 고유 ID |
| `station_name` | string | Y | 역명 |
| `line_no` | string | Y | 노선 번호 또는 노선명 |
| `lat` | double | N | 위도 |
| `lng` | double | N | 경도 |
| `address` | string | N | 역 주소. 지역명 검색 보조에 사용 |
| `transfer_lines` | string | N | 환승 가능 노선 목록 |

## 검증

- `station_id` 중복은 오류로 기록한다.
- `transfer_lines`는 쉼표 구분 문자열로 보관한다.
