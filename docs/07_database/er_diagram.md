# 파일 관계도

```mermaid
erDiagram
    BUS_STOPS {
        string stop_id PK
        string stop_name
        double lat
        double lng
        string district
    }

    BUS_ROUTES {
        string route_no PK
        string route_name
        string start_stop_id FK
        string end_stop_id FK
        int interval_min
    }

    ROUTE_STOPS {
        string route_no FK
        int seq
        string stop_id FK
        string direction
    }

    SUBWAY_STATIONS {
        string station_id PK
        string station_name
        string line_no
        double lat
        double lng
        string transfer_lines
    }

    FACILITIES {
        string facility_id PK
        string name
        string category
        string nearest_node_id
        string address
        double lat
        double lng
    }

    ARRIVAL_TEST {
        string route_no
        string node_id
        string time_slot
        int arrival_minutes
        string status
    }

    CROWDING_TEST {
        string route_no
        string node_id
        string time_slot
        string crowding_level
    }

    BUS_ROUTES ||--o{ ROUTE_STOPS : includes
    BUS_STOPS ||--o{ ROUTE_STOPS : appears_in
    BUS_STOPS ||--o{ FACILITIES : nearest
    SUBWAY_STATIONS ||--o{ FACILITIES : nearest
    BUS_ROUTES ||--o{ ARRIVAL_TEST : has
    BUS_ROUTES ||--o{ CROWDING_TEST : has
```

## 관계 해석

- `route_stops.route_no`는 `bus_routes.route_no`를 참조한다.
- `route_stops.stop_id`는 `bus_stops.stop_id`를 참조한다.
- `facilities.nearest_node_id`는 정류장 ID 또는 지하철역 ID를 가질 수 있다.
- `arrival_test.node_id`, `crowding_test.node_id`도 정류장 ID 또는 역 ID를 가질 수 있다.

