# 편의시설 패킷

## 요청/응답

```text
C->S  FACILITY_REQ|<node_keyword>:<category>
S->C  FACILITY_RES|<code>|<facility_name>:<category>:<nearest_node>:<address>;<facility_name>:...
```

## 필드

| 필드 | 설명 |
|------|------|
| `node_keyword` | 정류장명 또는 역명 |
| `category` | 시설 종류 |
| `facility_name` | 시설명 |
| `nearest_node` | 가장 가까운 정류장/역 |
| `address` | 주소 |

`category`는 `all`, `병원`, `약국`, `편의점`, `카페`, `PC방`, `음식점` 등을 사용할 수 있다.

## 결과 없음

해당 위치 또는 카테고리에 시설이 없으면 `FACILITY_RES|1|`을 반환한다.
