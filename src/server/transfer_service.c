#include "transfer_service.h"

#include "graph.h"
#include "protocol.h"
#include "string_utils.h"

#include <stdio.h>
#include <string.h>

static int node_has_coordinates(const GraphNode *node) {
    // 좌표가 모두 0인 데이터는 위치값이 누락된 항목으로 판단합니다.
    return node && node->lat != 0.0 && node->lng != 0.0;
}

// 입력한 위치 이름으로 그래프 안의 번호를 찾습니다.
static int resolve_node_by_keyword(AppData *data, const char *keyword) {
    int i;
    int candidate = -1;
    int coordinate_candidate = -1;
    int station_like;

    // 검색 기준과 노드 목록이 모두 준비된 경우에만 후보 탐색을 진행합니다.
    if (!data || str_is_blank(keyword)) {
        return -1;
    }

    // 노드 ID가 직접 입력된 경우를 먼저 처리하기 위해 정확 일치를 검사합니다.
    for (i = 0; i < data->graph.node_count; ++i) {
        if (strcmp(data->graph.nodes[i].node_id, keyword) == 0) {
            return i;
        }
    }

    // 입력에 '역'이 포함되면 지하철역 후보의 우선순위를 높입니다.
    station_like = keyword_match(keyword, "역");
    // 정확한 이름 일치를 기준으로 우선 후보를 검색합니다.
    for (i = 0; i < data->graph.node_count; ++i) {
        if (strcmp(data->graph.nodes[i].node_name, keyword) == 0) {
            // 역 검색이면 지하철역이 정확히 맞을 때 바로 반환합니다.
            if (station_like && strcmp(data->graph.nodes[i].node_type, "subway_station") == 0) {
                return i;
            }
            // 위치 정보가 있는 후보를 우선 저장합니다.
            if (coordinate_candidate < 0 && node_has_coordinates(&data->graph.nodes[i])) {
                coordinate_candidate = i;
            } else if (candidate < 0) {
                candidate = i;
            }
        }
    }
    if (coordinate_candidate >= 0) {
        return coordinate_candidate;
    }
    if (candidate >= 0) {
        return candidate;
    }

    // 정확 일치 후보가 확인되지 않을 때만 부분 일치 검색으로 범위를 넓힙니다.
    for (i = 0; i < data->graph.node_count; ++i) {
        if (keyword_match(data->graph.nodes[i].node_name, keyword)) {
            // 역 검색이면 지하철역 후보를 우선합니다.
            if (station_like && strcmp(data->graph.nodes[i].node_type, "subway_station") == 0) {
                return i;
            }
            // 위치 정보가 있는 후보를 먼저 저장합니다.
            if (coordinate_candidate < 0 && node_has_coordinates(&data->graph.nodes[i])) {
                coordinate_candidate = i;
            } else if (candidate < 0) {
                candidate = i;
            }
        }
    }
    if (coordinate_candidate >= 0) {
        return coordinate_candidate;
    }
    return candidate;
}

static int is_transfer_segment(const char *route_no) {
    // TRANSFER로 표시된 구간은 환승이나 도보 이동입니다.
    return route_no && strcmp(route_no, "TRANSFER") == 0;
}

// 경로 안에서 환승 횟수를 셉니다.
static int count_transfers(const GraphPath *path) {
    int i;
    int count = 0;
    int saw_transfer_bridge = 0;
    char last_ride[MAX_ROUTE_LEN] = "";

    // 경로가 없거나 노드가 하나뿐이면 환승이 없습니다.
    if (!path || path->count <= 1) {
        return 0;
    }

    // 경로의 각 구간을 보며 환승 여부를 셉니다.
    for (i = 0; i + 1 < path->count; ++i) {
        const char *route = path->segment_routes[i];
        // 환승/도보 구간은 바로 환승 횟수에 포함합니다.
        if (is_transfer_segment(route)) {
            count++;
            saw_transfer_bridge = 1;
            continue;
        }
        // 직전 이동 수단과 다르면 환승으로 봅니다.
        if (last_ride[0] && strcmp(last_ride, route) != 0 && !saw_transfer_bridge) {
            count++;
        }
        // 다음 구간 비교를 위해 현재 노선을 기억합니다.
        safe_strcpy(last_ride, sizeof(last_ride), route);
        saw_transfer_bridge = 0;
    }
    return count;
}

static double find_subway_distance_km(const AppData *data, const char *from_id, const char *to_id) {
    SubwayDistanceNode *cur;

    // 역간 거리 조회에 필요한 값이 부족하면 거리 정보를 0으로 둡니다.
    if (!data || !from_id || !to_id) {
        return 0.0;
    }
    // 역간 거리 목록에서 양방향으로 같은 구간을 찾습니다.
    for (cur = data->subway_distances; cur; cur = cur->next) {
        if ((strcmp(cur->from_station_id, from_id) == 0 && strcmp(cur->to_station_id, to_id) == 0) ||
            (strcmp(cur->from_station_id, to_id) == 0 && strcmp(cur->to_station_id, from_id) == 0)) {
            return cur->distance_km;
        }
    }
    return 0.0;
}

// 출발지와 도착지 입력을 최단 경로 응답으로 변환합니다.
int handle_transfer(AppData *data, const char *payload, char *response, unsigned long response_size) {
    char buf[256];
    char *from;
    char *to;
    int from_i;
    int to_i;
    GraphPath path;
    int i;
    int transfer_count;

    // 서비스 데이터나 요청 본문이 누락되면 오류 응답으로 분기합니다.
    if (!data || str_is_blank(payload)) {
        snprintf(response, response_size, "%s|%d:INVALID_REQUEST\n", PKT_ERROR, ERR_INVALID_REQUEST);
        return ERR_INVALID_REQUEST;
    }
    // 요청 본문을 분리하기 전에 작업용 버퍼로 복사합니다.
    safe_strcpy(buf, sizeof(buf), payload);
    from = buf;
    // 출발지와 도착지를 ':' 기준으로 나눕니다.
    to = strchr(buf, ITEM_SEP);
    if (!to) {
        snprintf(response, response_size, "%s|%d:INVALID_REQUEST\n", PKT_ERROR, ERR_INVALID_REQUEST);
        return ERR_INVALID_REQUEST;
    }
    *to = '\0';
    to++;
    // 입력값의 앞뒤 빈칸을 지웁니다.
    str_trim(from);
    str_trim(to);

    // 출발지와 도착지를 그래프 노드 번호로 바꿉니다.
    from_i = resolve_node_by_keyword(data, from);
    to_i = resolve_node_by_keyword(data, to);
    // 출발지·도착지 매칭 또는 최단 경로 탐색에 실패하면 결과 없음 응답을 반환합니다.
    if (from_i < 0 || to_i < 0 || !graph_find_path(&data->graph, from_i, to_i, &path)) {
        snprintf(response, response_size, "%s|%d|\n", PKT_TRANSFER_RES, ERR_NOT_FOUND);
        return ERR_NOT_FOUND;
    }

    // 응답 종류와 성공 코드를 먼저 버퍼에 구성합니다.
    snprintf(response, response_size, "%s|%d|", PKT_TRANSFER_RES, ERR_OK);
    // 경로 안의 환승 횟수를 계산합니다.
    transfer_count = count_transfers(&path);
    // 전체 경로의 노드 이름을 쉼표로 이어 붙입니다.
    for (i = 0; i < path.count; ++i) {
        if (i > 0) {
            str_append(response, response_size, ",");
        }
        str_append(response, response_size, data->graph.nodes[path.indices[i]].node_name);
    }
    // 환승 횟수와 총 소요 시간을 붙입니다.
    str_appendf(response, response_size, ":%d:%d:", transfer_count, path.total_minutes);
    // 각 구간 정보를 자세히 붙입니다.
    for (i = 0; i + 1 < path.count; ++i) {
        GraphNode *from_node = &data->graph.nodes[path.indices[i]];
        GraphNode *to_node = &data->graph.nodes[path.indices[i + 1]];
        const char *route_no = path.segment_routes[i][0] ? path.segment_routes[i] : "MOVE";
        double distance_km = 0.0;
        char distance_text[32] = "";

        // 지하철역 사이 이동이면 역간 거리 정보를 찾습니다.
        if (!is_transfer_segment(route_no) &&
            strcmp(from_node->node_type, "subway_station") == 0 &&
            strcmp(to_node->node_type, "subway_station") == 0) {
            distance_km = find_subway_distance_km(data, from_node->node_id, to_node->node_id);
            if (distance_km > 0.0) {
                snprintf(distance_text, sizeof(distance_text), "%.1f", distance_km);
            }
        }

        // 두 번째 구간부터는 ';'로 구분합니다.
        if (i > 0) {
            str_append(response, response_size, ";");
        }
        // 구간 정보를 '~'로 묶어 응답에 붙입니다.
        if (!str_appendf(response,
                         response_size,
                         "%s~%s~%s~%s~%s~%s~%s~%d~%s",
                         from_node->node_id,
                         from_node->node_name,
                         from_node->node_type,
                         route_no,
                         to_node->node_id,
                         to_node->node_name,
                         to_node->node_type,
                         path.segment_minutes[i],
                         distance_text)) {
            break;
        }
    }
    // 프로토콜 한 줄 응답을 완성하기 위해 마지막에 개행을 추가합니다.
    str_append(response, response_size, "\n");
    return ERR_OK;
}
