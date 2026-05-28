#include "operation_service.h"

#include "linked_list.h"
#include "protocol.h"
#include "string_utils.h"

#include <stdio.h>

// 도착 예정 시간을 찾습니다.
int handle_arrival(AppData *data, const char *payload, char *response, unsigned long response_size) {
    char node_id[32];
    char route_no[32];
    char time_slot[32];
    ArrivalNode *arrival;

    // 요청 내용은 노드ID:노선번호:시간대 형식이어야 합니다.
    if (!split_three_fields(payload,
                            ITEM_SEP,
                            node_id,
                            sizeof(node_id),
                            route_no,
                            sizeof(route_no),
                            time_slot,
                            sizeof(time_slot)) ||
        str_is_blank(node_id) || str_is_blank(route_no) || str_is_blank(time_slot)) {
        snprintf(response, response_size, "%s|%d:INVALID_REQUEST\n", PKT_ERROR, ERR_INVALID_REQUEST);
        return ERR_INVALID_REQUEST;
    }
    // 조건에 맞는 도착 정보를 찾습니다.
    arrival = find_arrival(data, node_id, route_no, time_slot);
    // 조회 조건과 일치하는 운행 데이터가 확인되지 않으면 결과 없음 코드를 반환합니다.
    if (!arrival) {
        snprintf(response, response_size, "%s|%d|\n", PKT_ARRIVAL_RES, ERR_NOT_FOUND);
        return ERR_NOT_FOUND;
    }
    // 조회된 도착 예정 시간 데이터를 프로토콜 응답 형식으로 구성합니다.
    snprintf(response, response_size, "%s|%d|%s:%s:%d:%s\n",
             PKT_ARRIVAL_RES,
             ERR_OK,
             arrival->route_no,
             find_node_name_by_id(data, arrival->node_id),
             arrival->arrival_minutes,
             arrival->status);
    return ERR_OK;
}

// 혼잡도 정보를 찾습니다.
int handle_crowding(AppData *data, const char *payload, char *response, unsigned long response_size) {
    char node_id[32];
    char route_no[32];
    char time_slot[32];
    CrowdingNode *crowding;

    // 요청 내용은 노드ID:노선번호:시간대 형식이어야 합니다.
    if (!split_three_fields(payload,
                            ITEM_SEP,
                            node_id,
                            sizeof(node_id),
                            route_no,
                            sizeof(route_no),
                            time_slot,
                            sizeof(time_slot)) ||
        str_is_blank(node_id) || str_is_blank(route_no) || str_is_blank(time_slot)) {
        snprintf(response, response_size, "%s|%d:INVALID_REQUEST\n", PKT_ERROR, ERR_INVALID_REQUEST);
        return ERR_INVALID_REQUEST;
    }
    // 조건에 맞는 혼잡도 정보를 찾습니다.
    crowding = find_crowding(data, node_id, route_no, time_slot);
    // 조회 조건과 일치하는 운행 데이터가 확인되지 않으면 결과 없음 코드를 반환합니다.
    if (!crowding) {
        snprintf(response, response_size, "%s|%d|\n", PKT_CROWDING_RES, ERR_NOT_FOUND);
        return ERR_NOT_FOUND;
    }
    // 조회된 혼잡도 데이터를 프로토콜 응답 형식으로 구성합니다.
    snprintf(response, response_size, "%s|%d|%s:%s:%s:%s\n",
             PKT_CROWDING_RES,
             ERR_OK,
             crowding->route_no,
             find_node_name_by_id(data, crowding->node_id),
             crowding->time_slot,
             crowding->crowding_level);
    return ERR_OK;
}
