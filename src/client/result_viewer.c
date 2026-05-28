#include "result_viewer.h"

#include "protocol.h"
#include "string_utils.h"
#include "types.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 서버 응답 헤더를 패킷 종류, 상태 코드, 본문으로 분리합니다.
static void split_header(const char *response, char *type, size_t type_size, int *code, char *payload, size_t payload_size) {
    char buf[MAX_PACKET_SIZE];
    char *p1;
    char *p2;

    // 파싱 과정에서 원본 문자열을 훼손하지 않도록 작업용 버퍼를 사용합니다.
    safe_strcpy(buf, sizeof(buf), response ? response : "");
    // 응답 파싱 전에 문자열 끝의 개행 문자를 제거합니다.
    strip_newline(buf);
    // 파싱 실패 상황을 대비해 기본 상태 코드는 서버 오류로 초기화합니다.
    type[0] = '\0';
    payload[0] = '\0';
    *code = ERR_SERVER_ERROR;

    // 첫 번째 필드 구분자를 기준으로 패킷 종류와 나머지 영역을 분리합니다.
    p1 = strchr(buf, FIELD_SEP);
    if (!p1) {
        safe_strcpy(type, type_size, buf);
        return;
    }
    // 분리된 패킷 종류를 호출자가 넘긴 버퍼에 저장합니다.
    *p1++ = '\0';
    safe_strcpy(type, type_size, buf);

    // ERROR 패킷은 상태 코드와 메시지 본문을 콜론 기준으로 분리합니다.
    if (strcmp(type, PKT_ERROR) == 0) {
        char *colon = strchr(p1, ITEM_SEP);
        if (colon) {
            *colon++ = '\0';
            *code = atoi(p1);
            safe_strcpy(payload, payload_size, colon);
        }
        return;
    }

    // 일반 패킷은 상태 코드와 본문을 필드 구분자 기준으로 분리합니다.
    p2 = strchr(p1, FIELD_SEP);
    if (!p2) {
        *code = atoi(p1);
        return;
    }
    *p2++ = '\0';
    *code = atoi(p1);
    safe_strcpy(payload, payload_size, p2);
}

// 화면 출력과 별개로 응답 상태 코드만 확인합니다.
int response_code(const char *response) {
    char type[64];
    char payload[MAX_PACKET_SIZE];
    int code;
    // 공통 헤더 파싱 함수를 이용해 상태 코드만 반환합니다.
    split_header(response, type, sizeof(type), &code, payload, sizeof(payload));
    return code;
}

// 검색 실패나 빈 결과 상황에서 공통 안내 문구를 출력합니다.
static void print_not_found(void) {
    printf("검색 결과가 없습니다.\n");
}

// 콜론과 세미콜론으로 구성된 목록형 응답을 라벨이 붙은 형식으로 출력합니다.
static void print_list_items(const char *payload, const char *label1, const char *label2, const char *label3, const char *label4) {
    char buf[MAX_PACKET_SIZE];
    char *item;
    int index = 1;

    // strtok가 문자열을 직접 수정하므로 응답 본문을 작업용 버퍼에 복사합니다.
    safe_strcpy(buf, sizeof(buf), payload);
    // 목록 응답에서 각 항목은 세미콜론 단위로 분리합니다.
    item = strtok(buf, ";");
    while (item) {
        char item_buf[1024];
        char *f1;
        char *f2;
        char *f3;
        char *f4;
        // 현재 항목을 별도 버퍼에 복사한 뒤 필드 단위로 분해합니다.
        safe_strcpy(item_buf, sizeof(item_buf), item);
        f1 = item_buf;
        // 첫 번째 콜론 위치를 찾아 첫 필드와 나머지 필드를 분리합니다.
        f2 = strchr(f1, ITEM_SEP);
        if (f2) {
            *f2++ = '\0';
        }
        // 남은 필드도 같은 방식으로 콜론 위치를 따라 분리합니다.
        f3 = f2 ? strchr(f2, ITEM_SEP) : NULL;
        if (f3) {
            *f3++ = '\0';
        }
        f4 = f3 ? strchr(f3, ITEM_SEP) : NULL;
        if (f4) {
            *f4++ = '\0';
        }
        // 분리된 필드 값을 전달된 라벨명과 함께 화면에 출력합니다.
        printf("[%d]\n", index++);
        printf("  %s: %s\n", label1, f1 ? f1 : "");
        if (f2) {
            printf("  %s: %s\n", label2, f2);
        }
        if (f3) {
            printf("  %s: %s\n", label3, f3);
        }
        if (f4 && label4) {
            printf("  %s: %s\n", label4, f4);
        }
        // 다음 세미콜론 항목을 이어서 처리합니다.
        item = strtok(NULL, ";");
    }
}

// 버스 노선 검색 응답을 노선 단위로 정리해 출력합니다.
static void show_bus_result(const char *payload) {
    char buf[MAX_PACKET_SIZE];
    char *item;
    int route_index = 1;

    // 버스 응답 본문을 수정 가능한 작업용 버퍼로 복사합니다.
    safe_strcpy(buf, sizeof(buf), payload);

    // 세미콜론으로 구분된 노선 항목을 순서대로 출력합니다.
    item = buf;
    while (item && *item) {
        char item_buf[2048];
        char stops_buf[2048];
        char *fields[5] = {0};
        char *cursor;
        char *next_item;
        char *route_id;
        char *route_name;
        char *start;
        char *end;
        char *stops;
        int field_count = 0;
        int stop_index = 1;

        // 현재 노선 항목의 끝과 다음 항목 시작 위치를 확인합니다.
        next_item = strchr(item, LIST_SEP);
        if (next_item) {
            *next_item++ = '\0';
        }

        // 노선 항목을 콜론 기준으로 세부 필드로 분리합니다.
        safe_strcpy(item_buf, sizeof(item_buf), item);
        cursor = item_buf;
        while (cursor && field_count < 5) {
            char *sep = strchr(cursor, ITEM_SEP);
            fields[field_count++] = cursor;
            if (!sep) {
                break;
            }
            *sep = '\0';
            cursor = sep + 1;
        }

        // 필드가 충분하면 노선 ID, 노선명, 출발, 종점, 정류장 목록으로 해석합니다.
        if (field_count >= 5) {
            route_id = fields[0];
            route_name = fields[1];
            start = fields[2];
            end = fields[3];
            stops = fields[4];
        } else if (field_count >= 4) {
            // 예전 형식처럼 노선명이 없는 경우에도 출력할 수 있게 맞춥니다.
            route_id = fields[0];
            route_name = fields[0];
            start = fields[1];
            end = fields[2];
            stops = fields[3];
        } else {
            // 예상 필드 수와 맞지 않는 항목은 원문 그대로 보여줍니다.
            printf("%s\n", item);
            item = next_item;
            continue;
        }

        // 노선 ID, 표시 이름, 출발지, 종점을 먼저 출력합니다.
        printf("[%d]\n", route_index++);
        printf("  노선ID: %s\n", route_id);
        printf("  노선명: %s\n", route_name);
        printf("  출발: %s\n", start);
        printf("  종점: %s\n", end);
        printf("  경유 정류장:\n");

        // 경유 정류장 문자열을 쉼표 단위로 나누어 목록 형태로 출력합니다.
        safe_strcpy(stops_buf, sizeof(stops_buf), stops);
        cursor = stops_buf;
        while (cursor && *cursor) {
            char *sep = strchr(cursor, ',');
            if (sep) {
                *sep++ = '\0';
            }
            // 각 경유 정류장에 순번을 붙여 표시합니다.
            printf("    %d. %s\n", stop_index++, cursor);
            cursor = sep;
        }

        item = next_item;
    }
}

// 그래프 내부 노드 타입을 사용자 화면용 한글 라벨로 변환합니다.
static const char *node_type_label(const char *type) {
    if (type && strcmp(type, "bus_stop") == 0) {
        return "정류장";
    }
    if (type && strcmp(type, "subway_station") == 0) {
        return "지하철역";
    }
    return "노드";
}

// 저장 파일의 종류 값을 사용자 화면용 한글 라벨로 변환합니다.
static const char *user_state_type_label(const char *type) {
    if (type && strcmp(type, "stop") == 0) {
        return "정류장";
    }
    if (type && strcmp(type, "bus") == 0) {
        return "버스";
    }
    if (type && strcmp(type, "station") == 0) {
        return "지하철역";
    }
    if (type && (strcmp(type, "transfer") == 0 || strcmp(type, "route") == 0)) {
        return "길찾기";
    }
    if (type && strcmp(type, "facility") == 0) {
        return "편의시설";
    }
    return type ? type : "";
}

// 서버 응답 본문이 안내 문구인 경우 별도 가공 없이 출력합니다.
static void print_message_payload(const char *payload) {
    if (payload && *payload) {
        printf("%s\n", payload);
    }
}

// 즐겨찾기 목록 응답을 번호가 붙은 화면 출력으로 변환합니다.
static void show_favorite_list_result(const char *payload) {
    char buf[MAX_PACKET_SIZE];
    char *item;
    int index = 1;

    if (str_is_blank(payload)) {
        // 응답 본문이 비어 있으면 즐겨찾기 목록이 비어 있다고 판단합니다.
        printf("저장된 즐겨찾기가 없습니다.\n");
        return;
    }

    // 즐겨찾기 목록 제목을 출력합니다.
    printf("----- 즐겨찾기 목록 -----\n");
    safe_strcpy(buf, sizeof(buf), payload);
    item = strtok(buf, ";");
    while (item) {
        char item_buf[1024];
        char *type;
        char *id;
        char *name;

        // type:id:name 형식으로 들어온 값을 나눕니다.
        safe_strcpy(item_buf, sizeof(item_buf), item);
        type = item_buf;
        id = strchr(type, ITEM_SEP);
        if (!id) {
            item = strtok(NULL, ";");
            continue;
        }
        *id++ = '\0';
        name = strchr(id, ITEM_SEP);
        if (!name) {
            item = strtok(NULL, ";");
            continue;
        }
        *name++ = '\0';

        // 목록 번호와 함께 한 줄로 출력합니다.
        printf("[%d] %s | %s | %s\n", index++, user_state_type_label(type), id, name);
        item = strtok(NULL, ";");
    }

    // 출력 가능한 항목을 찾지 못하면 빈 목록 안내를 출력합니다.
    if (index == 1) {
        printf("저장된 즐겨찾기가 없습니다.\n");
    }
}

// 최근 검색 기록을 화면에 출력합니다.
static void show_recent_list_result(const char *payload) {
    char buf[MAX_PACKET_SIZE];
    char *item;
    int index = 1;

    if (str_is_blank(payload)) {
        // 응답 본문이 비어 있으면 최근 검색 기록이 없다고 판단합니다.
        printf("최근 검색 기록이 없습니다.\n");
        return;
    }

    // 최근 검색 기록 제목을 출력합니다.
    printf("----- 최근 검색 기록 -----\n");
    safe_strcpy(buf, sizeof(buf), payload);
    item = strtok(buf, ";");
    while (item) {
        char item_buf[1024];
        char *type;
        char *keyword;
        char *ts;

        // type:keyword:time 형식으로 들어온 값을 나눕니다.
        safe_strcpy(item_buf, sizeof(item_buf), item);
        type = item_buf;
        keyword = strchr(type, ITEM_SEP);
        if (!keyword) {
            item = strtok(NULL, ";");
            continue;
        }
        *keyword++ = '\0';
        ts = strchr(keyword, ITEM_SEP);
        if (!ts) {
            item = strtok(NULL, ";");
            continue;
        }
        *ts++ = '\0';

        // 목록 번호와 검색 내용을 출력합니다.
        printf("[%d] %s | %s | %s\n", index++, user_state_type_label(type), keyword, ts);
        item = strtok(NULL, ";");
    }

    // 표시 가능한 최근 검색 항목이 확인되지 않을 경우 빈 기록 안내를 출력합니다.
    if (index == 1) {
        printf("최근 검색 기록이 없습니다.\n");
    }
}

// 이동 수단 이름을 화면에 보일 문구로 바꿉니다.
static void move_label(const char *route, const char *from_type, const char *to_type, char *out, size_t out_size) {
    // 환승 구간이면 환승/도보 이동으로 표시합니다.
    if (!route || strcmp(route, "TRANSFER") == 0) {
        safe_strcpy(out, out_size, "환승/도보 이동");
        return;
    }
    // 정류장끼리 이동하면 버스 번호를 붙여 보여줍니다.
    if (strcmp(from_type ? from_type : "", "bus_stop") == 0 &&
        strcmp(to_type ? to_type : "", "bus_stop") == 0) {
        snprintf(out, out_size, "버스 %s", route);
        return;
    }
    safe_strcpy(out, out_size, route);
}

// 길찾기 구간 하나를 '~' 기준으로 나눕니다.
static int split_step_fields(char *step, char **fields, int field_count) {
    int count = 0;
    char *cursor = step;

    // 필요한 필드 수만큼 반복해서 자릅니다.
    while (cursor && count < field_count) {
        char *sep = strchr(cursor, '~');
        fields[count++] = cursor;
        if (!sep) {
            break;
        }
        *sep = '\0';
        cursor = sep + 1;
    }
    return count;
}

// 전체 경로를 한 줄 요약으로 출력합니다.
static void print_path_summary(const char *path) {
    char buf[MAX_PACKET_SIZE];
    char *node;
    int first = 1;

    // 쉼표로 나누기 위해 임시 버퍼에 복사합니다.
    safe_strcpy(buf, sizeof(buf), path);
    printf("최단 경로 요약: ");
    // 쉼표로 이어진 경로 요약 문자열을 노드 단위로 분리합니다.
    node = strtok(buf, ",");
    while (node) {
        // 두 번째 노드부터 화살표를 붙입니다.
        if (!first) {
            printf(" -> ");
        }
        printf("%s", node);
        first = 0;
        // 다음 노드로 넘어갑니다.
        node = strtok(NULL, ",");
    }
    printf("\n");
}

// 길찾기 결과를 순서대로 출력합니다.
static void show_transfer_result(const char *payload) {
    char buf[MAX_PACKET_SIZE];
    char *path;
    char *count;
    char *minutes;
    char *steps;
    char steps_buf[MAX_PACKET_SIZE];
    char transfer_notes[MAX_PACKET_SIZE] = "";
    char last_ride[MAX_ROUTE_LEN] = "";
    int after_transfer_bridge = 0;
    int step_index = 1;

    // 응답 내용을 임시 버퍼로 복사합니다.
    safe_strcpy(buf, sizeof(buf), payload);
    path = buf;
    // 첫 번째 ':' 앞은 전체 경로 요약입니다.
    count = strchr(path, ITEM_SEP);
    if (!count) {
        printf("%s\n", payload);
        return;
    }
    *count++ = '\0';
    // 두 번째 값은 환승 횟수입니다.
    minutes = strchr(count, ITEM_SEP);
    if (!minutes) {
        printf("%s\n", payload);
        return;
    }
    *minutes++ = '\0';
    // 세 번째 값은 예상 소요시간이고, 그 뒤는 구간 목록입니다.
    steps = strchr(minutes, ITEM_SEP);
    if (steps) {
        *steps++ = '\0';
    }

    // 경로 요약, 환승 횟수, 예상 시간을 먼저 출력합니다.
    print_path_summary(path);
    printf("환승 횟수: %s\n", count);
    printf("예상 소요시간: %s분\n", minutes);

    // 세부 이동 구간이 누락된 경우 요약 정보까지만 출력합니다.
    if (!steps || !*steps) {
        return;
    }

    printf("\n구간별 이동:\n");
    // 구간 목록을 ';' 기준으로 나누어 출력합니다.
    safe_strcpy(steps_buf, sizeof(steps_buf), steps);
    for (char *step = strtok(steps_buf, ";"); step; step = strtok(NULL, ";")) {
        char step_item[1024];
        char *fields[9] = {0};
        char label[160];
        int field_count;
        const char *from_name;
        const char *from_type;
        const char *route;
        const char *to_name;
        const char *to_type;
        const char *step_minutes;
        const char *distance_km;

        // 구간 하나를 다시 복사해서 필드로 나눕니다.
        safe_strcpy(step_item, sizeof(step_item), step);
        field_count = split_step_fields(step_item, fields, 9);
        // 출력에 필요한 필드가 모자란 구간은 화면 표시에서 제외합니다.
        if (field_count < 8) {
            continue;
        }

        // 서버가 보낸 필드를 출력용 변수에 연결합니다.
        from_name = fields[1];
        from_type = fields[2];
        route = fields[3];
        to_name = fields[5];
        to_type = fields[6];
        step_minutes = fields[7];
        distance_km = field_count >= 9 ? fields[8] : "";
        // 이동 수단 이름을 화면 문구로 바꿉니다.
        move_label(route, from_type, to_type, label, sizeof(label));

        // 구간의 출발지와 도착지를 출력합니다.
        printf("  %d. %s(%s) -> %s(%s)\n",
               step_index++,
               from_name,
               node_type_label(from_type),
               to_name,
               node_type_label(to_type));
        printf("     이동수단: %s", label);
        // 거리 정보가 있으면 함께 출력합니다.
        if (distance_km && *distance_km) {
            printf(" / 거리 %skm", distance_km);
        }
        printf(" / 예상 %s분\n", step_minutes);

        // 환승/도보 구간이면 환승 안내에 추가합니다.
        if (strcmp(route, "TRANSFER") == 0) {
            str_appendf(transfer_notes,
                        sizeof(transfer_notes),
                        "  - %s에서 %s으로 환승/도보 이동\n",
                        from_name,
                        to_name);
            after_transfer_bridge = 1;
        } else {
            // 버스나 지하철 노선이 바뀌면 환승 안내에 추가합니다.
            if (last_ride[0] && strcmp(last_ride, route) != 0 && !after_transfer_bridge) {
                str_appendf(transfer_notes,
                            sizeof(transfer_notes),
                            "  - %s에서 %s -> %s 환승\n",
                            from_name,
                            last_ride,
                            route);
            }
            // 다음 구간 비교를 위해 현재 이동 수단을 저장합니다.
            safe_strcpy(last_ride, sizeof(last_ride), route);
            after_transfer_bridge = 0;
        }
    }

    // 모아 둔 환승 안내가 있으면 마지막에 출력합니다.
    if (transfer_notes[0]) {
        printf("\n환승 지점:\n%s", transfer_notes);
    }
}

// 서버 응답 종류에 맞춰 결과를 보여줍니다.
void result_viewer_show(const char *response) {
    char type[64];
    char payload[MAX_PACKET_SIZE];
    int code;

    // 서버 응답 헤더를 패킷 종류, 상태 코드, 본문으로 분리합니다.
    split_header(response, type, sizeof(type), &code, payload, sizeof(payload));

    // 오류 응답이면 오류 문구를 출력합니다.
    if (strcmp(type, PKT_ERROR) == 0) {
        printf("오류: %s\n", payload[0] ? payload : "요청을 처리할 수 없습니다.");
        return;
    }
    // 즐겨찾기 추가/삭제 응답은 서버 문구를 바로 보여줍니다.
    if ((strcmp(type, PKT_FAVORITE_ADD_RES) == 0 ||
         strcmp(type, PKT_FAVORITE_DELETE_RES) == 0) &&
        payload[0]) {
        print_message_payload(payload);
        return;
    }
    // 검색 결과가 없다는 코드이면 공통 문구를 출력합니다.
    if (code == ERR_NOT_FOUND) {
        print_not_found();
        return;
    }
    // 정상 코드가 아니면 코드 번호를 보여줍니다.
    if (code != ERR_OK) {
        printf("오류 코드 %d\n", code);
        return;
    }

    // 응답 종류에 따라 출력 함수를 고릅니다.
    if (strcmp(type, PKT_STOP_SEARCH_RES) == 0) {
        printf("----- 정류장 검색 결과 -----\n");
        print_list_items(payload, "정류장 ID", "정류장명", "경유 버스", NULL);
    } else if (strcmp(type, PKT_BUS_SEARCH_RES) == 0) {
        printf("----- 버스 노선 정보 -----\n");
        show_bus_result(payload);
    } else if (strcmp(type, PKT_SUBWAY_SEARCH_RES) == 0) {
        printf("----- 지하철역 검색 결과 -----\n");
        print_list_items(payload, "역 ID", "역명", "노선", "환승 가능 노선");
    } else if (strcmp(type, PKT_TRANSFER_RES) == 0) {
        printf("----- 길찾기 최단 경로 -----\n");
        show_transfer_result(payload);
    } else if (strcmp(type, PKT_FACILITY_RES) == 0) {
        printf("----- 주변 편의시설 -----\n");
        print_list_items(payload, "시설명", "종류", "가까운 노드", "주소");
    } else if (strcmp(type, PKT_ARRIVAL_RES) == 0) {
        print_list_items(payload, "노선", "노드", "도착 예정(분)", "상태");
    } else if (strcmp(type, PKT_CROWDING_RES) == 0) {
        print_list_items(payload, "노선", "노드", "시간대", "혼잡도");
    } else if (strcmp(type, PKT_FAVORITE_ADD_RES) == 0 ||
               strcmp(type, PKT_FAVORITE_DELETE_RES) == 0) {
        print_message_payload(payload);
    } else if (strcmp(type, PKT_FAVORITE_LIST_RES) == 0) {
        show_favorite_list_result(payload);
    } else if (strcmp(type, PKT_RECENT_LIST_RES) == 0) {
        show_recent_list_result(payload);
    } else {
        printf("%s", response);
    }
}
