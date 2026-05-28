#ifndef TRANSIT_TYPES_H
#define TRANSIT_TYPES_H

#include <stddef.h>

// 문자열 길이와 배열 크기를 한 곳에서 정합니다.
#define MAX_ID_LEN 32
#define MAX_NAME_LEN 100
#define MAX_CATEGORY_LEN 50
#define MAX_ADDRESS_LEN 200
#define MAX_ROUTE_LEN 128
#define MAX_STATUS_LEN 50
#define MAX_TIME_SLOT_LEN 32
#define MAX_NODE_TYPE_LEN 16
#define MAX_CLIENTS 32
#define MAX_GRAPH_PATH 256

// 버스 정류장 한 개를 저장합니다.
typedef struct StopNode {
    char stop_id[MAX_ID_LEN];        // 정류장 ID
    char stop_name[MAX_NAME_LEN];    // 정류장 이름
    double lat;                      // 위도
    double lng;                      // 경도
    char district[MAX_CATEGORY_LEN]; // 지역명
    struct StopNode *next;           // 다음 정류장
} StopNode;

// 버스 노선에 포함된 정류장 한 개를 저장합니다.
typedef struct RouteStopNode {
    char route_no[MAX_ROUTE_LEN];       // 노선 번호
    int seq;                            // 정류장 순서
    char stop_id[MAX_ID_LEN];           // 정류장 ID
    char direction[MAX_CATEGORY_LEN];   // 운행 방향
    struct RouteStopNode *next;         // 다음 노선 정류장
} RouteStopNode;

// 버스 노선 한 개를 저장합니다.
typedef struct RouteNode {
    char route_no[MAX_ROUTE_LEN];      // 노선 번호
    char route_name[MAX_NAME_LEN];     // 노선 이름
    char start_stop_id[MAX_ID_LEN];    // 출발 정류장 ID
    char end_stop_id[MAX_ID_LEN];      // 종점 정류장 ID
    int interval_min;                  // 배차 간격
    RouteStopNode *stops;              // 이 노선이 지나는 정류장 목록
    struct RouteNode *next;            // 다음 노선
} RouteNode;

// 지하철역 한 개를 저장합니다.
typedef struct StationNode {
    char station_id[MAX_ID_LEN];        // 역 ID
    char station_name[MAX_NAME_LEN];    // 역 이름
    char line_no[MAX_ROUTE_LEN];        // 노선명
    double lat;                         // 위도
    double lng;                         // 경도
    char address[MAX_ADDRESS_LEN];      // 주소
    char transfer_lines[MAX_NAME_LEN];  // 환승 가능 노선
    struct StationNode *next;           // 다음 역
} StationNode;

// 지하철 역 사이의 거리 정보를 저장합니다.
typedef struct SubwayDistanceNode {
    char from_station_id[MAX_ID_LEN];   // 출발역 ID
    char to_station_id[MAX_ID_LEN];     // 도착역 ID
    char line_no[MAX_ROUTE_LEN];        // 노선명
    double distance_km;                 // 거리(km)
    int estimated_minutes;              // 예상 시간
    struct SubwayDistanceNode *next;    // 다음 거리 정보
} SubwayDistanceNode;

// 주변 편의시설 한 개를 저장합니다.
typedef struct FacilityNode {
    char facility_id[MAX_ID_LEN];       // 시설 ID
    char name[MAX_NAME_LEN];            // 시설 이름
    char category[MAX_CATEGORY_LEN];    // 시설 종류
    char nearest_node_id[MAX_ID_LEN];   // 가까운 정류장 또는 역 ID
    char address[MAX_ADDRESS_LEN];      // 주소
    double lat;                         // 위도
    double lng;                         // 경도
    struct FacilityNode *next;          // 다음 시설
} FacilityNode;

// 도착 예정 시간 테스트 데이터를 저장합니다.
typedef struct ArrivalNode {
    char route_no[MAX_ROUTE_LEN];       // 노선 번호
    char node_id[MAX_ID_LEN];           // 정류장 또는 역 ID
    char time_slot[MAX_TIME_SLOT_LEN];  // 시간대
    int arrival_minutes;                // 도착 예정 시간
    char status[MAX_STATUS_LEN];        // 운행 상태
    struct ArrivalNode *next;           // 다음 도착 정보
} ArrivalNode;

// 혼잡도 테스트 데이터를 저장합니다.
typedef struct CrowdingNode {
    char route_no[MAX_ROUTE_LEN];       // 노선 번호
    char node_id[MAX_ID_LEN];           // 정류장 또는 역 ID
    char time_slot[MAX_TIME_SLOT_LEN];  // 시간대
    char crowding_level[MAX_STATUS_LEN];// 혼잡도
    struct CrowdingNode *next;          // 다음 혼잡도 정보
} CrowdingNode;

// 그래프에서 한 노드가 다른 노드로 가는 길을 저장합니다.
typedef struct GraphEdge {
    int to_index;                       // 도착 노드 번호
    int weight_min;                     // 이동 시간
    char route_no[MAX_ROUTE_LEN];       // 이동 노선
    struct GraphEdge *next;             // 다음 간선
} GraphEdge;

// 그래프의 노드 한 개를 저장합니다.
typedef struct GraphNode {
    char node_id[MAX_ID_LEN];           // 정류장 또는 역 ID
    char node_name[MAX_NAME_LEN];       // 정류장 또는 역 이름
    char node_type[MAX_NODE_TYPE_LEN];  // 정류장/지하철역 구분
    double lat;                         // 위도
    double lng;                         // 경도
    GraphEdge *edges;                   // 연결된 이동 구간 목록
} GraphNode;

// 환승 경로 탐색에 쓰는 그래프입니다.
typedef struct TransferGraph {
    GraphNode *nodes; // 노드 배열
    int node_count;   // 현재 노드 수
    int capacity;     // 확보한 노드 배열 크기
} TransferGraph;

// 서버가 메모리에 들고 있는 전체 데이터입니다.
typedef struct AppData {
    StopNode *stops;
    RouteNode *routes;
    StationNode *stations;
    SubwayDistanceNode *subway_distances;
    FacilityNode *facilities;
    ArrivalNode *arrivals;
    CrowdingNode *crowdings;
    TransferGraph graph;
} AppData;

// 클라이언트 접속 정보를 저장합니다.
typedef struct ClientSession {
    int fd;             // 소켓 번호
    char client_ip[64]; // 클라이언트 IP
    int active;         // 사용 여부
} ClientSession;

// 서버 실행에 필요한 상태를 모아 둡니다.
typedef struct ServerContext {
    int listen_fd;                         // 서버 소켓
    int port;                              // 서버 포트
    AppData data;                          // 로드된 교통 데이터
    ClientSession sessions[MAX_CLIENTS];   // 접속 정보 배열
} ServerContext;

// 그래프 탐색 결과 경로를 저장합니다.
typedef struct GraphPath {
    int indices[MAX_GRAPH_PATH];                         // 경로에 포함된 노드 번호
    char segment_routes[MAX_GRAPH_PATH][MAX_ROUTE_LEN];  // 구간별 이동 노선
    int segment_minutes[MAX_GRAPH_PATH];                 // 구간별 이동 시간
    int count;                                           // 경로 노드 수
    int total_minutes;                                   // 총 이동 시간
} GraphPath;

#endif
