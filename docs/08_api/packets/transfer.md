# 길찾기 패킷

## 요청/응답

```text
C->S  TRANSFER_REQ|<from_node_keyword>:<to_node_keyword>
S->C  TRANSFER_RES|<code>|<path_node1,path_node2,...>:<transfer_count>:<estimated_minutes>:<step1;step2;...>
```

## 필드

| 필드 | 설명 |
|------|------|
| `from_node_keyword` | 탑승 위치 정류장명 또는 역명 |
| `to_node_keyword` | 도착 정류장명 또는 역명 |
| `path_node` | 경로에 포함된 정류장/역 표시명 |
| `transfer_count` | 환승 횟수 |
| `estimated_minutes` | 예상 소요시간 |
| `step` | `from_id~from_name~from_type~route_no~to_id~to_name~to_type~minutes~distance_km` 형식의 구간 상세 |

`from_type`과 `to_type`은 `bus_stop`, `subway_station` 중 하나다. `route_no`가 `TRANSFER`이면 정류장과 역 사이의 환승/도보 이동 구간을 의미한다.
서버는 그래프 가중치 기반 최단 경로를 찾아 응답한다.
`distance_km`은 지하철 인접역 이동일 때 채우며, 버스/환승 구간에서는 빈 값일 수 있다.

## 예

```text
TRANSFER_REQ|천안역:아산역
TRANSFER_RES|0|천안역,봉명역,쌍용역,아산역:0:9:ST004~천안역~subway_station~수도권 전철 1호선~ST005~봉명역~subway_station~3~1.3;ST005~봉명역~subway_station~수도권 전철 1호선~ST006~쌍용역~subway_station~3~1.6;ST006~쌍용역~subway_station~수도권 전철 1호선~ST007~아산역~subway_station~3~1.6
```
