#include "facility_service.h"

#include "protocol.h"
#include "string_utils.h"

#include <stdio.h>
#include <string.h>

// 입력한 이름에 맞는 정류장이나 역을 찾습니다.
static const char *resolve_node_id(AppData *data, const char *keyword) {
    int i;
    int candidate = -1;
    int station_like;

    // 검색 기준과 노드 목록이 모두 준비된 경우에만 후보 탐색을 진행합니다.
    if (!data || str_is_blank(keyword)) {
        return NULL;
    }

    // 노드 ID가 직접 입력된 경우를 먼저 처리하기 위해 정확 일치를 검사합니다.
    for (i = 0; i < data->graph.node_count; ++i) {
        if (strcmp(data->graph.nodes[i].node_id, keyword) == 0) {
            return data->graph.nodes[i].node_id;
        }
    }

    // 입력에 '역'이 포함되면 지하철역 후보의 우선순위를 높입니다.
    station_like = keyword_match(keyword, "역");
    // 정확한 이름 일치를 기준으로 우선 후보를 검색합니다.
    for (i = 0; i < data->graph.node_count; ++i) {
        if (strcmp(data->graph.nodes[i].node_name, keyword) == 0) {
            if (station_like && strcmp(data->graph.nodes[i].node_type, "subway_station") == 0) {
                return data->graph.nodes[i].node_id;
            }
            // 지하철역 우선 조건이 아니면 첫 번째 후보를 저장합니다.
            if (candidate < 0) {
                candidate = i;
            }
        }
    }
    if (candidate >= 0) {
        return data->graph.nodes[candidate].node_id;
    }

    // 정확 일치 후보가 없을 때만 부분 일치 검색으로 범위를 넓힙니다.
    for (i = 0; i < data->graph.node_count; ++i) {
        if (keyword_match(data->graph.nodes[i].node_name, keyword)) {
            if (candidate < 0 ||
                (station_like && strcmp(data->graph.nodes[i].node_type, "subway_station") == 0)) {
                // 지하철역을 찾는 상황이면 지하철역 후보를 우선합니다.
                candidate = i;
                if (station_like && strcmp(data->graph.nodes[i].node_type, "subway_station") == 0) {
                    break;
                }
            }
        }
    }
    if (candidate >= 0) {
        return data->graph.nodes[candidate].node_id;
    }
    return NULL;
}

// 요청 위치와 카테고리 조건에 맞는 편의시설 목록 응답을 구성합니다.
int handle_facility(AppData *data, const char *payload, char *response, unsigned long response_size) {
    char buf[256];
    char *node_keyword;
    char *category;
    const char *node_id;
    FacilityNode *cur;
    int count = 0;

    // 서비스 데이터나 요청 본문이 누락되면 오류 응답으로 분기합니다.
    if (!data || str_is_blank(payload)) {
        snprintf(response, response_size, "%s|%d:INVALID_REQUEST\n", PKT_ERROR, ERR_INVALID_REQUEST);
        return ERR_INVALID_REQUEST;
    }
    // 요청 본문을 분리하기 전에 작업용 버퍼로 복사합니다.
    safe_strcpy(buf, sizeof(buf), payload);
    node_keyword = buf;
    // 위치 이름과 카테고리를 ':' 기준으로 나눕니다.
    category = strchr(buf, ITEM_SEP);
    if (!category) {
        snprintf(response, response_size, "%s|%d:INVALID_REQUEST\n", PKT_ERROR, ERR_INVALID_REQUEST);
        return ERR_INVALID_REQUEST;
    }
    *category = '\0';
    category++;
    // 양쪽 입력값의 앞뒤 빈칸을 지웁니다.
    str_trim(node_keyword);
    str_trim(category);

    // 입력한 위치 이름을 그래프 노드 ID로 바꿉니다.
    node_id = resolve_node_id(data, node_keyword);
    if (!node_id) {
        snprintf(response, response_size, "%s|%d|\n", PKT_FACILITY_RES, ERR_NOT_FOUND);
        return ERR_NOT_FOUND;
    }

    // 응답 종류와 성공 코드를 먼저 버퍼에 구성합니다.
    snprintf(response, response_size, "%s|%d|", PKT_FACILITY_RES, ERR_OK);
    // 편의시설 목록을 돌면서 가까운 노드가 같은 항목을 찾습니다.
    for (cur = data->facilities; cur; cur = cur->next) {
        // 카테고리를 지정하지 않았거나 all이면 시설 종류 필터를 적용하지 않습니다.
        if (strcmp(cur->nearest_node_id, node_id) == 0 &&
            (str_is_blank(category) || strcmp(category, "all") == 0 || keyword_match(cur->category, category))) {
            // 응답 크기가 넘치거나 10개를 넘으면 중단합니다.
            if (strlen(response) + strlen(cur->name) + strlen(cur->category) +
                    strlen(cur->nearest_node_id) + strlen(cur->address) + 16 >= response_size ||
                count >= 10) {
                break;
            }
            if (count > 0) {
                str_append(response, response_size, ";");
            }
            // 시설명, 종류, 가까운 노드, 주소를 응답에 붙입니다.
            str_appendf(response, response_size, "%s:%s:%s:%s",
                        cur->name,
                        cur->category,
                        cur->nearest_node_id,
                        cur->address);
            count++;
        }
    }
    // 시설 필터를 통과한 항목이 없을 경우 결과 없음 코드를 설정합니다.
    if (count == 0) {
        snprintf(response, response_size, "%s|%d|\n", PKT_FACILITY_RES, ERR_NOT_FOUND);
        return ERR_NOT_FOUND;
    }
    // 프로토콜 한 줄 응답을 완성하기 위해 마지막에 개행을 추가합니다.
    str_append(response, response_size, "\n");
    return ERR_OK;
}
