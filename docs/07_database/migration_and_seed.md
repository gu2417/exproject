# 데이터 준비 및 테스트 데이터

## 1. 공공데이터 CSV 준비

`data/` 폴더에 다음 파일을 둔다.

```text
data/bus_stops.csv
data/bus_routes.csv
data/route_stops.csv
data/subway_stations.csv
data/subway_distances.csv
data/facilities.csv
data/arrival_test.csv
data/crowding_test.csv
```

## 2. 테스트 데이터 생성 원칙

- 실시간 API를 사용하지 않으므로 도착 예정시간과 혼잡도는 테스트 CSV로 보완한다.
- 테스트 데이터는 실제 노선/정류장 ID와 연결되어야 한다.
- 시연용 시간대는 `morning`, `day`, `evening`, `night`처럼 고정 문자열을 사용할 수 있다.

## 3. 최소 시드 데이터

최소 시연을 위해 다음 조건을 만족해야 한다.

| 데이터 | 최소 조건 |
|--------|-----------|
| 정류장 | 3개 이상 |
| 노선 | 2개 이상 |
| 노선별 정류장 | 각 노선당 2개 이상 |
| 지하철역 | 2개 이상 |
| 지하철 역간거리 | 인접역 1개 구간 이상 |
| 편의시설 | 3개 이상 |
| 운행 테스트 | 검색 가능한 노선/노드 조합 포함 |
