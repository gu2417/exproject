# 검색 패킷

## 정류장 검색

```text
C->S  STOP_SEARCH_REQ|<keyword>
S->C  STOP_SEARCH_RES|<code>|<stop_id>:<stop_name>:<route_no,route_no,...>;<stop_id>:...
```

## 버스 번호 검색

```text
C->S  BUS_SEARCH_REQ|<route_no_or_route_name>
S->C  BUS_SEARCH_RES|<code>|<route_no>:<route_name>:<start_stop>:<end_stop>:<stop1,stop2,stop3,...>;<route_no>:...
```

`route_no_or_route_name`에는 내부 노선 ID뿐 아니라 `311`, `311번`, `980`, `502번` 같은 실제 버스 번호 또는 노선명을 입력할 수 있다. 여러 노선이 일치하면 최대 10건을 반환한다.

## 지하철역 검색

```text
C->S  SUBWAY_SEARCH_REQ|<keyword>
S->C  SUBWAY_SEARCH_RES|<code>|<station_id>:<station_name>:<line_no>:<transfer_lines>;<station_id>:...
```

## 코드

| code | 의미 |
|------|------|
| 0 | 검색 성공 |
| 1 | 결과 없음 |
| 2 | 잘못된 요청 |
| 4 | 서버 오류 |
