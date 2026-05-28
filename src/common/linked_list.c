#include "linked_list.h"

#include "graph.h"
#include "string_utils.h"

#include <stdlib.h>
#include <string.h>

// 전체 데이터 저장 공간을 초기화합니다.
void app_data_init(AppData *data) {
    // 초기화 대상 구조체가 유효할 때만 메모리 값을 정리합니다.
    if (!data) {
        return;
    }
    // 구조체 전체를 0으로 채웁니다.
    memset(data, 0, sizeof(*data));
    // 환승 그래프도 따로 초기화합니다.
    graph_init(&data->graph);
}

// 버스 노선 안의 정류장 목록을 지웁니다.
static void free_route_stops(RouteStopNode *node) {
    // 연결 리스트를 따라가며 하나씩 해제합니다.
    while (node) {
        RouteStopNode *next = node->next;
        free(node);
        node = next;
    }
}

// 프로그램에서 읽어 둔 전체 데이터를 해제합니다.
void app_data_free(AppData *data) {
    // 해제할 데이터 구조체가 유효하지 않으면 정리 루틴을 종료합니다.
    if (!data) {
        return;
    }

    // 정류장 목록을 모두 지웁니다.
    while (data->stops) {
        StopNode *next = data->stops->next;
        free(data->stops);
        data->stops = next;
    }
    // 버스 노선과 노선별 정류장 목록을 모두 지웁니다.
    while (data->routes) {
        RouteNode *next = data->routes->next;
        free_route_stops(data->routes->stops);
        free(data->routes);
        data->routes = next;
    }
    // 지하철역 목록을 모두 지웁니다.
    while (data->stations) {
        StationNode *next = data->stations->next;
        free(data->stations);
        data->stations = next;
    }
    // 역간 거리 목록을 모두 지웁니다.
    while (data->subway_distances) {
        SubwayDistanceNode *next = data->subway_distances->next;
        free(data->subway_distances);
        data->subway_distances = next;
    }
    // 편의시설 목록을 모두 지웁니다.
    while (data->facilities) {
        FacilityNode *next = data->facilities->next;
        free(data->facilities);
        data->facilities = next;
    }
    // 도착 예정 시간 목록을 모두 지웁니다.
    while (data->arrivals) {
        ArrivalNode *next = data->arrivals->next;
        free(data->arrivals);
        data->arrivals = next;
    }
    // 혼잡도 목록을 모두 지웁니다.
    while (data->crowdings) {
        CrowdingNode *next = data->crowdings->next;
        free(data->crowdings);
        data->crowdings = next;
    }
    // 마지막으로 환승 그래프를 지웁니다.
    graph_free(&data->graph);
}

// 정류장 데이터를 목록 앞에 추가합니다.
StopNode *stop_add(AppData *data, const StopNode *value) {
    StopNode *node;
    // 저장소와 원본 데이터가 모두 유효할 때만 새 노드를 생성합니다.
    if (!data || !value) {
        return NULL;
    }
    // 정류장 정보를 저장할 연결 리스트 노드를 할당합니다.
    node = (StopNode *)calloc(1, sizeof(*node));
    if (!node) {
        return NULL;
    }
    // 받은 값을 새 노드에 복사합니다.
    *node = *value;
    // 목록 맨 앞에 연결합니다.
    node->next = data->stops;
    data->stops = node;
    return node;
}

// 버스 노선 데이터를 목록 앞에 추가합니다.
RouteNode *route_add(AppData *data, const RouteNode *value) {
    RouteNode *node;
    // 저장소와 원본 데이터가 모두 유효할 때만 새 노드를 생성합니다.
    if (!data || !value) {
        return NULL;
    }
    // 버스 노선 정보를 저장할 연결 리스트 노드를 할당합니다.
    node = (RouteNode *)calloc(1, sizeof(*node));
    if (!node) {
        return NULL;
    }
    // 노선 값을 복사하고 정류장 목록은 비워 둡니다.
    *node = *value;
    node->stops = NULL;
    node->next = data->routes;
    data->routes = node;
    return node;
}

// 노선 안에 정류장을 순서에 맞게 추가합니다.
RouteStopNode *route_stop_add(RouteNode *route, const RouteStopNode *value) {
    RouteStopNode *node;
    RouteStopNode **cur;
    // 노선과 정류장 값이 모두 준비되어야 노선별 정류장 노드를 추가합니다.
    if (!route || !value) {
        return NULL;
    }
    // 노선 안의 정류장 순서를 저장할 노드를 할당합니다.
    node = (RouteStopNode *)calloc(1, sizeof(*node));
    if (!node) {
        return NULL;
    }
    *node = *value;
    node->next = NULL;

    // seq 값이 작은 순서가 되도록 들어갈 위치를 찾습니다.
    cur = &route->stops;
    while (*cur && (*cur)->seq <= node->seq) {
        cur = &(*cur)->next;
    }
    // 찾은 위치에 새 노드를 끼워 넣습니다.
    node->next = *cur;
    *cur = node;
    return node;
}

// 지하철역 데이터를 목록 앞에 추가합니다.
StationNode *station_add(AppData *data, const StationNode *value) {
    StationNode *node;
    // 저장소와 원본 데이터가 모두 유효할 때만 새 노드를 생성합니다.
    if (!data || !value) {
        return NULL;
    }
    // 지하철역 정보를 저장할 연결 리스트 노드를 할당합니다.
    node = (StationNode *)calloc(1, sizeof(*node));
    if (!node) {
        return NULL;
    }
    // 값을 복사하고 목록 앞에 붙입니다.
    *node = *value;
    node->next = data->stations;
    data->stations = node;
    return node;
}

// 지하철 역간 거리 데이터를 추가합니다.
SubwayDistanceNode *subway_distance_add(AppData *data, const SubwayDistanceNode *value) {
    SubwayDistanceNode *node;
    // 저장소와 원본 데이터가 모두 유효할 때만 새 노드를 생성합니다.
    if (!data || !value) {
        return NULL;
    }
    // 역간 거리 정보를 저장할 연결 리스트 노드를 할당합니다.
    node = (SubwayDistanceNode *)calloc(1, sizeof(*node));
    if (!node) {
        return NULL;
    }
    *node = *value;
    node->next = data->subway_distances;
    data->subway_distances = node;
    return node;
}

// 편의시설 데이터를 목록 앞에 추가합니다.
FacilityNode *facility_add(AppData *data, const FacilityNode *value) {
    FacilityNode *node;
    // 저장소와 원본 데이터가 모두 유효할 때만 새 노드를 생성합니다.
    if (!data || !value) {
        return NULL;
    }
    // 편의시설 정보를 저장할 연결 리스트 노드를 할당합니다.
    node = (FacilityNode *)calloc(1, sizeof(*node));
    if (!node) {
        return NULL;
    }
    *node = *value;
    node->next = data->facilities;
    data->facilities = node;
    return node;
}

// 도착 예정 시간 데이터를 추가합니다.
ArrivalNode *arrival_add(AppData *data, const ArrivalNode *value) {
    ArrivalNode *node;
    // 저장소와 원본 데이터가 모두 유효할 때만 새 노드를 생성합니다.
    if (!data || !value) {
        return NULL;
    }
    // 도착 예정 시간 정보를 저장할 연결 리스트 노드를 할당합니다.
    node = (ArrivalNode *)calloc(1, sizeof(*node));
    if (!node) {
        return NULL;
    }
    *node = *value;
    node->next = data->arrivals;
    data->arrivals = node;
    return node;
}

// 혼잡도 데이터를 추가합니다.
CrowdingNode *crowding_add(AppData *data, const CrowdingNode *value) {
    CrowdingNode *node;
    // 저장소와 원본 데이터가 모두 유효할 때만 새 노드를 생성합니다.
    if (!data || !value) {
        return NULL;
    }
    // 혼잡도 정보를 저장할 연결 리스트 노드를 할당합니다.
    node = (CrowdingNode *)calloc(1, sizeof(*node));
    if (!node) {
        return NULL;
    }
    *node = *value;
    node->next = data->crowdings;
    data->crowdings = node;
    return node;
}

// 정류장 ID로 정류장을 찾습니다.
StopNode *find_stop_by_id(const AppData *data, const char *stop_id) {
    StopNode *cur;
    // 검색 대상 목록과 ID가 준비된 경우에만 정류장 조회를 수행합니다.
    if (!data || !stop_id) {
        return NULL;
    }
    // 정류장 목록을 차례대로 확인합니다.
    for (cur = data->stops; cur; cur = cur->next) {
        if (strcmp(cur->stop_id, stop_id) == 0) {
            return cur;
        }
    }
    return NULL;
}

// 노선 번호로 버스 노선을 찾습니다.
RouteNode *find_route_by_no(const AppData *data, const char *route_no) {
    RouteNode *cur;
    // 노선 목록과 노선 번호가 준비된 경우에만 노선 조회를 수행합니다.
    if (!data || !route_no) {
        return NULL;
    }
    // 노선 목록을 차례대로 확인합니다.
    for (cur = data->routes; cur; cur = cur->next) {
        if (strcmp(cur->route_no, route_no) == 0) {
            return cur;
        }
    }
    return NULL;
}

// 역 ID로 지하철역을 찾습니다.
StationNode *find_station_by_id(const AppData *data, const char *station_id) {
    StationNode *cur;
    // 역 목록과 역 ID가 준비된 경우에만 지하철역 조회를 수행합니다.
    if (!data || !station_id) {
        return NULL;
    }
    // 역 목록을 차례대로 확인합니다.
    for (cur = data->stations; cur; cur = cur->next) {
        if (strcmp(cur->station_id, station_id) == 0) {
            return cur;
        }
    }
    return NULL;
}

// 노드 ID에 맞는 정류장명이나 역명을 찾습니다.
const char *find_node_name_by_id(const AppData *data, const char *node_id) {
    // 먼저 정류장 ID로 찾아봅니다.
    StopNode *stop = find_stop_by_id(data, node_id);
    StationNode *station;
    if (stop) {
        return stop->stop_name;
    }
    // 정류장이 아니면 지하철역 ID로 찾아봅니다.
    station = find_station_by_id(data, node_id);
    if (station) {
        return station->station_name;
    }
    // 이름을 못 찾으면 원래 ID를 그대로 보여줍니다.
    return node_id ? node_id : "";
}

// 도착 예정 시간 데이터를 찾습니다.
ArrivalNode *find_arrival(const AppData *data, const char *node_id, const char *route_no, const char *time_slot) {
    ArrivalNode *cur;
    // 조회할 데이터 목록이 준비되지 않으면 탐색을 진행하지 않습니다.
    if (!data) {
        return NULL;
    }
    // 노드, 노선, 시간대가 모두 같은 항목을 찾습니다.
    for (cur = data->arrivals; cur; cur = cur->next) {
        if (strcmp(cur->node_id, node_id) == 0 &&
            strcmp(cur->route_no, route_no) == 0 &&
            strcmp(cur->time_slot, time_slot) == 0) {
            return cur;
        }
    }
    return NULL;
}

// 혼잡도 데이터를 찾습니다.
CrowdingNode *find_crowding(const AppData *data, const char *node_id, const char *route_no, const char *time_slot) {
    CrowdingNode *cur;
    // 조회할 데이터 목록이 준비되지 않으면 탐색을 진행하지 않습니다.
    if (!data) {
        return NULL;
    }
    // 노드, 노선, 시간대가 모두 같은 항목을 찾습니다.
    for (cur = data->crowdings; cur; cur = cur->next) {
        if (strcmp(cur->node_id, node_id) == 0 &&
            strcmp(cur->route_no, route_no) == 0 &&
            strcmp(cur->time_slot, time_slot) == 0) {
            return cur;
        }
    }
    return NULL;
}

// 해당 노선이 특정 정류장을 지나는지 확인합니다.
int route_has_stop(const RouteNode *route, const char *stop_id) {
    const RouteStopNode *cur;
    // 노선 정보나 정류장 ID가 부족하면 해당 노선을 경유 후보에서 제외합니다.
    if (!route || !stop_id) {
        return 0;
    }
    // 노선의 정류장 목록을 차례대로 확인합니다.
    for (cur = route->stops; cur; cur = cur->next) {
        if (strcmp(cur->stop_id, stop_id) == 0) {
            return 1;
        }
    }
    return 0;
}
