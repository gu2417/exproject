# 운행 정보 패킷

## 도착 예정시간

```text
C->S  ARRIVAL_REQ|<node_id>:<route_no>:<time_slot>
S->C  ARRIVAL_RES|<code>|<route_no>:<node_name>:<arrival_minutes>:<status>
```

## 혼잡도

```text
C->S  CROWDING_REQ|<node_id>:<route_no>:<time_slot>
S->C  CROWDING_RES|<code>|<route_no>:<node_name>:<time_slot>:<crowding_level>
```

## 값

| 필드 | 예 |
|------|----|
| `time_slot` | `morning`, `day`, `evening`, `night` |
| `status` | `운행 시작`, `곧 도착`, `운행 종료` |
| `crowding_level` | `여유`, `보통`, `혼잡` |

운행 정보는 실시간 API가 아니라 테스트 CSV를 기반으로 한다.

