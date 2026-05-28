# 데이터 로딩 및 캐시 수명

DB 연결 풀은 사용하지 않는다. 대신 서버 시작 시 CSV를 한 번 로드하고,
인메모리 구조를 요청 처리 동안 재사용한다.

## 1. 로딩 순서

1. `data/bus_stops.csv`
2. `data/bus_routes.csv`
3. `data/route_stops.csv`
4. `data/subway_stations.csv`
5. `data/subway_distances.csv`
6. `data/facilities.csv`
7. `data/arrival_test.csv`
8. `data/crowding_test.csv`
9. 그래프 생성

## 2. 파일 핸들 정책

- CSV 파일은 로딩 중에만 열고 즉시 닫는다.
- 로그 파일은 기록 시 열고 닫거나, 서버 실행 중 핸들을 유지할 수 있다.
- 클라이언트 로컬 파일은 메뉴 작업 단위로 열고 닫는다.

## 3. 캐시 정책

| 데이터 | 수명 | 변경 여부 |
|--------|------|-----------|
| 교통 CSV 데이터 | 서버 실행 전체 | 변경 없음 |
| 환승 그래프 | 서버 실행 전체 | 변경 없음 |
| 서버 로그 | 실행 중 계속 append | 변경 |
| 즐겨찾기 | 클라이언트 실행 중 파일 기준 | 변경 |
| 최근 검색 | 클라이언트 실행 중 파일 기준 | 변경 |
