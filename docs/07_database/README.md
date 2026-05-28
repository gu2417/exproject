# 07 - Database

현재 프로젝트는 DBMS를 사용하지 않고 CSV/텍스트 파일을 데이터 저장소로 사용한다.
이 섹션은 `docs_ref`의 database 양식을 유지하되, 파일 기반 스키마와 인메모리 조회 구조를 설명한다.

| 문서 | 내용 |
|------|------|
| [er_diagram.md](./er_diagram.md) | CSV 파일 간 관계도 |
| [query_catalog.md](./query_catalog.md) | 기능별 조회 로직 |
| [connection_pooling.md](./connection_pooling.md) | 데이터 로딩, 캐시, 파일 핸들 수명 |
| [migration_and_seed.md](./migration_and_seed.md) | CSV 준비와 테스트 데이터 생성 |
| [indexes_and_performance.md](./indexes_and_performance.md) | 연결리스트/그래프 검색 성능 전략 |

## 파일 스키마

| 파일 | 문서 |
|------|------|
| `bus_stops.csv` | [tables/bus_stops.md](./tables/bus_stops.md) |
| `bus_routes.csv` | [tables/bus_routes.md](./tables/bus_routes.md) |
| `route_stops.csv` | [tables/route_stops.md](./tables/route_stops.md) |
| `subway_stations.csv` | [tables/subway_stations.md](./tables/subway_stations.md) |
| `subway_distances.csv` | [tables/subway_distances.md](./tables/subway_distances.md) |
| `facilities.csv` | [tables/facilities.md](./tables/facilities.md) |
| `arrival_test.csv` | [tables/arrival_test.md](./tables/arrival_test.md) |
| `crowding_test.csv` | [tables/crowding_test.md](./tables/crowding_test.md) |
| `favorites.txt` | [tables/favorites.md](./tables/favorites.md) |
| `recent_search.txt` | [tables/recent_search.md](./tables/recent_search.md) |
