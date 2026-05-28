# arrival_test.csv

도착 예정시간과 운행 상태 테스트 데이터다.

| 컬럼 | 타입 | 필수 | 설명 |
|------|------|------|------|
| `route_no` | string | Y | 버스 번호 또는 노선 번호 |
| `node_id` | string | Y | 정류장/역 ID |
| `time_slot` | string | Y | 시간대 |
| `arrival_minutes` | int | Y | 도착 예정 분 |
| `status` | string | Y | 운행 상태 |

## 상태값

| 값 | 의미 |
|----|------|
| `운행 시작` | 운행이 시작됨 |
| `곧 도착` | 짧은 시간 내 도착 예정 |
| `운행 종료` | 해당 시간대 운행 종료 |

