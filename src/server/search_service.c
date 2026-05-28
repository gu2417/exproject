#include "search_service.h"

#include "linked_list.h"
#include "protocol.h"
#include "string_utils.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STOP_SEARCH_MAX_RESULTS 100
#define STOP_ROUTES_TEXT_SIZE 4096

// STOP_번호 형식에서 번호만 뽑습니다.
static int parse_stop_alias_number(const char *stop_id, long *out) {
    char *end;

    // 예상한 접두어 형식과 다르면 추출 실패로 반환합니다.
    if (!stop_id || strncmp(stop_id, "STOP_", 5) != 0 || !isdigit((unsigned char)stop_id[5])) {
        return 0;
    }
    // STOP_ 뒤의 숫자 부분을 long 값으로 바꿉니다.
    end = NULL;
    *out = strtol(stop_id + 5, &end, 10);
    return end && *end == '\0';
}

// 숫자형 정류장 ID의 뒤쪽 번호를 뽑습니다.
static int parse_node_tail_number(const char *stop_id, long *out) {
    const char *tail;
    char *end;
    size_t len;

    // 정류장 ID가 비어 있으면 번호 추출을 시도하지 않습니다.
    if (!stop_id) {
        return 0;
    }
    len = strlen(stop_id);
    // 앞의 세 자리를 뺄 수 있을 만큼 길어야 합니다.
    if (len <= 3) {
        return 0;
    }
    // 전체가 숫자인지 확인합니다.
    for (size_t i = 0; i < len; ++i) {
        if (!isdigit((unsigned char)stop_id[i])) {
            return 0;
        }
    }
    // 앞의 세 자리를 제외하고 숫자로 바꿉니다.
    tail = stop_id + 3;
    end = NULL;
    *out = strtol(tail, &end, 10);
    return end && *end == '\0';
}

// 서로 다른 정류장 ID가 같은 정류장을 가리키는지 확인합니다.
static int stop_ids_alias_match(const char *alias_id, const char *node_id) {
    long alias_no;
    long node_no;

    // 두 형식에서 번호를 뽑지 못하면 같은 것으로 보지 않습니다.
    if (!parse_stop_alias_number(alias_id, &alias_no) ||
        !parse_node_tail_number(node_id, &node_no)) {
        return 0;
    }
    return alias_no == node_no;
}

// 문자열이 지정한 글자로 시작하는지 확인합니다.
static int starts_with_text(const char *value, const char *prefix) {
    size_t prefix_len;

    // 비교할 노선 문자열이 비어 있으면 필터 불일치로 판단합니다.
    if (!value || !prefix) {
        return 0;
    }
    prefix_len = strlen(prefix);
    return strncmp(value, prefix, prefix_len) == 0;
}

// 긴 노선 ID에서 실제 버스 번호처럼 보이는 뒤쪽 값을 뽑습니다.
static int route_number_from_route_id(const char *route_no, char *out, size_t out_size) {
    const char *suffix;

    // 입력 문자열과 출력 버퍼가 모두 있어야 번호 추출을 진행합니다.
    if (!route_no || !out || out_size == 0) {
        return 0;
    }
    out[0] = '\0';

    // 아산/천안 쪽 노선 ID 앞자리를 확인합니다.
    if (starts_with_text(route_no, "285000") ||
        starts_with_text(route_no, "288000") ||
        starts_with_text(route_no, "298000")) {
        suffix = route_no + 6;
        // 앞쪽 0은 화면에 보이지 않게 넘깁니다.
        while (*suffix == '0' && suffix[1]) {
            suffix++;
        }
        safe_strcpy(out, out_size, suffix);
        return !str_is_blank(out);
    }
    return 0;
}

// 실제 운행 노선명으로 보기 어려운 이름인지 확인합니다.
static int route_name_is_inactive(const char *name) {
    // 이름이 비어 있으면 제외합니다.
    if (str_is_blank(name)) {
        return 1;
    }
    // 테스트나 수정 표시가 들어간 노선은 검색 결과에서 제외합니다.
    return strstr(name, "테스트") ||
           strstr(name, "노선명 미상") ||
           strstr(name, "미상") ||
           strstr(name, "신규") ||
           strstr(name, "수정") ||
           strstr(name, "추가") ||
           strstr(name, "삭제") ||
           strstr(name, "변경") ||
           strstr(name, "개편") ||
           strstr(name, "적용") ||
           strstr(name, "링크") ||
           strstr(name, "정류장") ||
           strstr(name, "회차지") ||
           strstr(name, "상행") ||
           strstr(name, "하행") ||
           strstr(name, "미경유") ||
           strstr(name, "노선") ||
           strchr(name, '_');
}

// '마중 1', '순환 2'처럼 접두어가 붙은 노선명에서 표시용 번호를 추출합니다.
static int copy_prefixed_route_number(const char *name,
                                      const char *prefix,
                                      const char *label_prefix,
                                      int append_beon,
                                      char *out,
                                      size_t out_size) {
    const char *pos;
    const char *p;
    char number[32];
    size_t used = 0;

    // 지정한 접두어가 들어 있는 위치를 찾습니다.
    pos = strstr(name, prefix);
    if (!pos) {
        return 0;
    }
    // 접두어 뒤쪽부터 번호를 찾습니다.
    p = pos + strlen(prefix);
    while (*p && isspace((unsigned char)*p)) {
        p++;
    }
    // 번호로 시작하지 않으면 이 형식이 아닙니다.
    if (!isdigit((unsigned char)*p)) {
        return 0;
    }
    // 숫자와 하이픈으로 된 번호 부분을 복사합니다.
    while (*p && used + 1 < sizeof(number) && (isdigit((unsigned char)*p) || *p == '-')) {
        number[used++] = *p++;
    }
    number[used] = '\0';
    if (used == 0) {
        return 0;
    }
    while (*p && isspace((unsigned char)*p)) {
        p++;
    }
    // 뒤에 '번'이 붙어 있으면 건너뜁니다.
    if (starts_with_text(p, "번")) {
        p += strlen("번");
    }
    while (*p && isspace((unsigned char)*p)) {
        p++;
    }
    // 번호 뒤에 다른 글자가 남아 있으면 제외합니다.
    if (*p) {
        return 0;
    }
    // 사용자에게 보여줄 노선 표시 이름을 정규화합니다.
    snprintf(out, out_size, append_beon ? "%s%s번" : "%s%s", label_prefix, number);
    return 1;
}

// 숫자가 날짜처럼 보이는지 확인합니다.
static int looks_like_date_number(const char *digits, int digit_count, const char *after) {
    int month;
    int day;

    // 여섯 자리 이상이면 날짜나 관리 번호일 가능성이 높습니다.
    if (digit_count >= 6) {
        return 1;
    }
    // 네 자리 뒤에 '_'나 '-'가 있으면 월일 형식인지 봅니다.
    if (digit_count == 4 && (*after == '_' || *after == '-')) {
        month = (digits[0] - '0') * 10 + (digits[1] - '0');
        day = (digits[2] - '0') * 10 + (digits[3] - '0');
        if (month >= 1 && month <= 12 && day >= 1 && day <= 31) {
            return 1;
        }
    }
    // 네 자리 숫자 중 일부는 연도처럼 보이면 제외합니다.
    if (digit_count == 4 &&
        (digits[0] == '0' ||
         (digits[0] == '1' && digits[1] == '8') ||
         (digits[0] == '2' && digits[1] == '0'))) {
        return 1;
    }
    return starts_with_text(after, "월") || starts_with_text(after, "일");
}

// 노선명 맨 앞의 숫자를 버스 번호로 뽑습니다.
static int extract_numeric_route_number(const char *name, char *out, size_t out_size) {
    const char *p = name;
    char token[32];
    char digits[32];
    int digit_count = 0;
    size_t token_used = 0;

    // 노선명이 비어 있으면 표시 번호 추출을 진행하지 않습니다.
    if (!p) {
        return 0;
    }
    // 토큰 시작 위치가 나올 때까지 선행 공백을 이동합니다.
    while (*p && isspace((unsigned char)*p)) {
        p++;
    }
    // 숫자로 시작하지 않으면 이 형식이 아닙니다.
    if (!isdigit((unsigned char)*p)) {
        return 0;
    }
    // 번호로 쓸 수 있는 글자를 모읍니다.
    while (*p && token_used + 1 < sizeof(token) && (isdigit((unsigned char)*p) || *p == '-')) {
        token[token_used++] = *p;
        if (isdigit((unsigned char)*p) && digit_count + 1 < (int)sizeof(digits)) {
            digits[digit_count++] = *p;
        }
        p++;
    }
    token[token_used] = '\0';
    digits[digit_count] = '\0';

    // 숫자가 없거나 너무 길거나 날짜처럼 보이면 제외합니다.
    if (digit_count == 0 || digit_count > 4 || looks_like_date_number(digits, digit_count, p)) {
        return 0;
    }
    // 뒤에 '번'이 붙으면 건너뜁니다.
    if (starts_with_text(p, "번")) {
        p += strlen("번");
    }
    while (*p && isspace((unsigned char)*p)) {
        p++;
    }
    // 번호 뒤에 다른 글자가 남으면 제외합니다.
    if (*p) {
        return 0;
    }

    // 화면에는 '번'을 붙여서 보여줍니다.
    snprintf(out, out_size, "%s번", token);
    return 1;
}

// 노선 번호와 노선명을 바탕으로 화면 표시용 이름을 구성합니다.
static int route_display_name(const RouteNode *route, char *out, size_t out_size) {
    const char *name;

    // 노선 정보와 출력 버퍼가 준비된 경우에만 표시 이름을 구성합니다.
    if (!route || !out || out_size == 0) {
        return 0;
    }
    out[0] = '\0';
    // 노선명이 비어 있을 경우 노선 번호를 표시 이름으로 대체합니다.
    name = str_is_blank(route->route_name) ? route->route_no : route->route_name;
    // 실제 운행 노선으로 보기 어려우면 제외합니다.
    if (route_name_is_inactive(name)) {
        return 0;
    }

    // 이미 방면 표시가 있는 노선은 번호와 함께 보여줍니다.
    if (strstr(name, "방면")) {
        char route_number[32];

        if (route_number_from_route_id(route->route_no, route_number, sizeof(route_number))) {
            snprintf(out, out_size, "%s번 - %s", route_number, name);
        } else {
            snprintf(out, out_size, "노선ID %s - %s", route->route_no, name);
        }
        return 1;
    }

    // 여러 노선명 형식을 차례대로 확인합니다.
    if (copy_prefixed_route_number(name, "마중", "마중", 0, out, out_size) ||
        copy_prefixed_route_number(name, "순환", "순환", 1, out, out_size) ||
        copy_prefixed_route_number(name, "순", "순환", 1, out, out_size) ||
        copy_prefixed_route_number(name, "공영", "공영 ", 0, out, out_size) ||
        extract_numeric_route_number(name, out, out_size)) {
        return 1;
    }

    return 0;
}

static int route_display_label(AppData *data, const RouteNode *route, char *out, size_t out_size) {
    char base[MAX_ROUTE_LEN];
    const char *destination;

    // 중복 검사와 화면 출력을 위해 기본 노선 표시 이름을 먼저 구성합니다.
    if (!route_display_name(route, base, sizeof(base))) {
        return 0;
    }
    // 이미 방면 정보가 있으면 그대로 사용합니다.
    if (strstr(base, "방면")) {
        safe_strcpy(out, out_size, base);
        return 1;
    }

    // 종점 이름을 찾아 방면 정보로 붙입니다.
    destination = data ? find_node_name_by_id(data, route->end_stop_id) : "";
    if (!str_is_blank(destination) &&
        strcmp(destination, route->end_stop_id) != 0 &&
        !strstr(destination, "정류장명 미상")) {
        snprintf(out, out_size, "%s - %s 방면", base, destination);
    } else {
        // 종점 이름을 제대로 찾지 못하면 기본 이름만 사용합니다.
        safe_strcpy(out, out_size, base);
    }
    return 1;
}

// 이미 넣은 노선 표시인지 확인합니다.
static int route_key_exists(const char *keys, const char *route_key) {
    char needle[MAX_ROUTE_LEN + 4];

    // 비교 문자열이 비어 있으면 중복 후보가 아니라고 판단합니다.
    if (!keys || !route_key || !*route_key) {
        return 0;
    }
    // |노선| 형태로 찾아서 부분 문자열 착각을 줄입니다.
    snprintf(needle, sizeof(needle), "|%s|", route_key);
    return strstr(keys, needle) != NULL;
}

// 버스 검색 결과에서 중복 노선을 막기 위해 표시해 둡니다.
static int mark_bus_route_seen(AppData *data, const RouteNode *route, char *keys, unsigned long keys_size) {
    char label[MAX_ROUTE_LEN];
    char key[MAX_ROUTE_LEN + MAX_NAME_LEN * 2 + 8];
    const char *start_name;
    const char *end_name;

    // 노선 표시를 만들 핵심 값이 부족하면 해당 항목을 출력하지 않습니다.
    if (!data || !route || !keys) {
        return 0;
    }
    // 사용자 화면에 표시할 노선명을 정규화합니다.
    if (!route_display_label(data, route, label, sizeof(label))) {
        return 0;
    }

    // 노선명, 출발지, 종점을 조합해 중복 판단용 키를 구성합니다.
    start_name = find_node_name_by_id(data, route->start_stop_id);
    end_name = find_node_name_by_id(data, route->end_stop_id);
    snprintf(key, sizeof(key), "%s>%s>%s", label, start_name, end_name);
    // 이미 나온 노선이면 다시 넣지 않습니다.
    if (route_key_exists(keys, key)) {
        return 0;
    }
    // 새 노선이면 키 목록에 추가합니다.
    return str_appendf(keys, keys_size, "|%s|", key);
}

// 정류장 검색 결과에 노선 하나를 추가합니다.
static int append_route_item(AppData *data,
                             const char *stop_id,
                             const RouteNode *route,
                             char *routes,
                             unsigned long routes_size,
                             char *route_keys,
                             unsigned long route_keys_size) {
    ArrivalNode *arrival;
    CrowdingNode *crowding;
    char label[MAX_ROUTE_LEN];

    // 출력에 필요한 핵심 값이 부족한 항목은 응답에서 제외합니다.
    if (!route || !stop_id || !routes || !route_keys) {
        return 0;
    }

    // 사용자 화면에 표시할 노선명을 정규화합니다.
    if (!route_display_label(data, route, label, sizeof(label))) {
        return 0;
    }
    // 같은 노선명이 이미 들어갔으면 건너뜁니다.
    if (route_key_exists(route_keys, label)) {
        return 0;
    }
    // 중복 확인용 목록에 노선명을 저장합니다.
    if (!str_appendf(route_keys, route_keys_size, "|%s|", label)) {
        return 0;
    }

    // 오전 기준 테스트 도착 정보와 혼잡도 정보를 찾아봅니다.
    arrival = find_arrival(data, stop_id, route->route_no, "morning");
    crowding = find_crowding(data, stop_id, route->route_no, "morning");
    // 기존 노선이 있으면 쉼표로 구분합니다.
    if (routes[0]) {
        str_append(routes, routes_size, ",");
    }
    // 도착 정보나 혼잡도가 있으면 괄호 안에 같이 표시합니다.
    if (arrival || crowding) {
        str_appendf(routes, routes_size, "%s(", label);
        if (arrival) {
            // 도착 시간이 있으면 분과 상태를 적습니다.
            str_appendf(routes, routes_size, "%d분/%s", arrival->arrival_minutes, arrival->status);
        } else {
            // 도착 시간 정보가 누락된 경우 별도 안내 문구를 붙입니다.
            str_append(routes, routes_size, "도착정보없음");
        }
        if (crowding) {
            // 혼잡도 정보가 있으면 뒤에 붙입니다.
            str_appendf(routes, routes_size, "/%s", crowding->crowding_level);
        }
        str_append(routes, routes_size, ")");
    } else {
        // 도착 시간이나 혼잡도 정보가 없을 때는 노선명만 출력 대상으로 사용합니다.
        str_append(routes, routes_size, label);
    }
    return 1;
}

// 특정 정류장을 지나는 노선을 모읍니다.
static int append_routes_for_stop_id(AppData *data,
                                     const char *stop_id,
                                     char *routes,
                                     unsigned long routes_size,
                                     char *route_keys,
                                     unsigned long route_keys_size) {
    RouteNode *route;
    int count = 0;

    // 모든 노선을 돌며 해당 정류장을 지나는지 확인합니다.
    for (route = data->routes; route; route = route->next) {
        if (route_has_stop(route, stop_id)) {
            if (append_route_item(data, stop_id, route, routes, routes_size, route_keys, route_keys_size)) {
                count++;
            }
        }
    }
    return count;
}

// 같은 정류장을 다른 ID로 가진 항목의 노선도 찾습니다.
static int append_routes_for_alias_stop_ids(AppData *data,
                                            const StopNode *stop,
                                            char *routes,
                                            unsigned long routes_size,
                                            char *route_keys,
                                            unsigned long route_keys_size) {
    StopNode *other;
    int count = 0;

    // 기준 정류장과 노선 목록이 준비된 경우에만 경유 노선을 탐색합니다.
    if (!data || !stop) {
        return 0;
    }

    // 다른 정류장 ID 중 같은 번호로 볼 수 있는 항목을 찾습니다.
    for (other = data->stops; other; other = other->next) {
        if (other == stop || strcmp(other->stop_id, stop->stop_id) == 0) {
            continue;
        }
        if (stop_ids_alias_match(stop->stop_id, other->stop_id)) {
            // 같은 정류장으로 보이면 그 ID 기준 노선도 모읍니다.
            count += append_routes_for_stop_id(data, other->stop_id, routes, routes_size, route_keys, route_keys_size);
        }
    }
    return count;
}

// 정류장 검색 결과에 붙일 경유 노선 목록 문자열을 구성합니다.
static void append_routes_for_stop(AppData *data,
                                   const StopNode *stop,
                                   char *routes,
                                   unsigned long routes_size) {
    char route_keys[STOP_ROUTES_TEXT_SIZE];
    int count;

    // 결과 문자열과 중복 확인용 문자열을 비웁니다.
    routes[0] = '\0';
    route_keys[0] = '\0';
    // 정류장 정보나 전체 데이터가 유효하지 않을 때는 경유 노선 표시를 '-'로 대체합니다.
    if (!data || !stop) {
        safe_strcpy(routes, routes_size, "-");
        return;
    }

    // 먼저 현재 정류장 ID 기준으로 노선을 찾습니다.
    count = append_routes_for_stop_id(data, stop->stop_id, routes, routes_size, route_keys, sizeof(route_keys));

    // 현재 ID에서 노선을 찾지 못하면 같은 번호를 공유하는 정류장 ID까지 확장해 확인합니다.
    if (count == 0) {
        count += append_routes_for_alias_stop_ids(data, stop, routes, routes_size, route_keys, sizeof(route_keys));
    }

    // 확장 검색에서도 노선이 확인되지 않으면 화면 표시값을 '-'로 확정합니다.
    if (count == 0 || !routes[0]) {
        safe_strcpy(routes, routes_size, "-");
    }
}

// 정류장 이름 또는 ID 검색 결과를 프로토콜 응답으로 구성합니다.
int handle_stop_search(AppData *data, const char *keyword, char *response, unsigned long response_size) {
    StopNode *cur;
    int count = 0;

    // 검색어가 비어 있거나 통신 구분자를 포함하면 요청 오류로 처리합니다.
    if (!data || str_is_blank(keyword) || str_contains_protocol_delim(keyword)) {
        snprintf(response, response_size, "%s|%d:INVALID_REQUEST\n", PKT_ERROR, ERR_INVALID_REQUEST);
        return ERR_INVALID_REQUEST;
    }

    // 응답 종류와 성공 코드를 먼저 버퍼에 구성합니다.
    snprintf(response, response_size, "%s|%d|", PKT_STOP_SEARCH_RES, ERR_OK);
    // 전체 정류장을 돌며 이름이나 ID가 맞는 항목을 찾습니다.
    for (cur = data->stops; cur; cur = cur->next) {
        if (keyword_match(cur->stop_name, keyword) || strcmp(cur->stop_id, keyword) == 0) {
            char routes[STOP_ROUTES_TEXT_SIZE];
            size_t needed;

            // 최대 출력 개수를 넘기지 않습니다.
            if (count >= STOP_SEARCH_MAX_RESULTS) {
                break;
            }
            // 해당 정류장을 경유하는 노선 목록을 별도 문자열로 정리합니다.
            append_routes_for_stop(data, cur, routes, sizeof(routes));
            // 응답에 들어갈 대략적인 길이를 계산합니다.
            needed = strlen(cur->stop_id) + strlen(cur->stop_name) + strlen(routes) + 4;
            if (strlen(response) + needed >= response_size) {
                break;
            }
            // 두 번째 항목부터는 ';'로 구분합니다.
            if (count > 0) {
                str_append(response, response_size, ";");
            }
            // 정류장 ID, 이름, 경유 노선을 응답에 붙입니다.
            str_appendf(response, response_size, "%s:%s:%s", cur->stop_id, cur->stop_name, routes);
            count++;
        }
    }
    // 검색 조건을 만족하는 항목이 없을 때 결과 없음 응답을 구성합니다.
    if (count == 0) {
        snprintf(response, response_size, "%s|%d|\n", PKT_STOP_SEARCH_RES, ERR_NOT_FOUND);
        return ERR_NOT_FOUND;
    }
    // 프로토콜 한 줄 응답을 완성하기 위해 마지막에 개행을 추가합니다.
    str_append(response, response_size, "\n");
    return ERR_OK;
}

static void compact_spaces(const char *src, char *dst, size_t dst_size) {
    size_t used = 0;
    // 결과를 기록할 버퍼가 유효하지 않으면 정리 작업을 수행하지 않습니다.
    if (!dst || dst_size == 0) {
        return;
    }
    dst[0] = '\0';
    // 원본 문자열이 없을 때는 결과를 빈 문자열로 초기화합니다.
    if (!src) {
        return;
    }
    // 빈칸을 빼고 글자만 복사합니다.
    while (*src && used + 1 < dst_size) {
        if (!isspace((unsigned char)*src)) {
            dst[used++] = *src;
        }
        src++;
    }
    dst[used] = '\0';
}

// 문자열 맨 앞의 숫자만 따로 복사합니다.
static void leading_digits(const char *src, char *dst, size_t dst_size) {
    size_t used = 0;
    // 결과를 기록할 버퍼가 유효하지 않으면 정리 작업을 수행하지 않습니다.
    if (!dst || dst_size == 0) {
        return;
    }
    dst[0] = '\0';
    // 원본 문자열이 없을 때는 결과를 빈 문자열로 초기화합니다.
    if (!src) {
        return;
    }
    // 숫자가 이어지는 동안만 복사합니다.
    while (*src && isdigit((unsigned char)*src) && used + 1 < dst_size) {
        dst[used++] = *src++;
    }
    dst[used] = '\0';
}

// 버스 검색어와 노선의 맞는 정도를 계산합니다.
static int route_match_rank(const RouteNode *route, const char *keyword) {
    char query[128];
    char name[256];
    char query_digits[64];
    char name_digits[64];
    char route_id_number[64];

    // 노선이 없거나 검색어가 비었거나 제외할 노선명이면 맞지 않습니다.
    if (!route || str_is_blank(keyword) || route_name_is_inactive(route->route_name)) {
        return 0;
    }
    // 노선 번호나 노선명이 정확히 같으면 가장 높은 순위입니다.
    if (strcmp(route->route_no, keyword) == 0 || strcmp(route->route_name, keyword) == 0) {
        return 1;
    }

    // 빈칸을 제거한 뒤 다시 비교합니다.
    compact_spaces(keyword, query, sizeof(query));
    compact_spaces(route->route_name, name, sizeof(name));
    if (strcmp(name, query) == 0) {
        return 1;
    }

    // 검색어와 노선명 앞쪽 숫자를 비교합니다.
    leading_digits(query, query_digits, sizeof(query_digits));
    leading_digits(name, name_digits, sizeof(name_digits));
    if (query_digits[0]) {
        // 앞 숫자가 같으면 두 번째 순위입니다.
        if (name_digits[0] && strcmp(query_digits, name_digits) == 0) {
            return 2;
        }
        // 이름에는 숫자가 없지만 노선 ID에서 번호를 뽑을 수 있으면 비교합니다.
        if (!name_digits[0] &&
            strstr(route->route_name, "방면") &&
            route_number_from_route_id(route->route_no, route_id_number, sizeof(route_id_number)) &&
            strcmp(query_digits, route_id_number) == 0) {
            return 2;
        }
        // 숫자 검색인데 맞지 않으면 제외합니다.
        return 0;
    }

    // 일반 글자 검색은 노선명 부분 일치로 확인합니다.
    return keyword_match(route->route_name, keyword) ? 3 : 0;
}

// 버스 검색 결과에 노선 하나를 붙입니다.
static int append_bus_route_item(AppData *data, RouteNode *route, char *response, unsigned long response_size, int item_index) {
    RouteStopNode *cur;
    int count = 0;
    const char *start_name;
    const char *end_name;
    char route_label[MAX_ROUTE_LEN];

    // 노선 데이터가 완전한 경우에만 검색 응답에 항목을 추가합니다.
    if (!data || !route) {
        return 0;
    }
    // 사용자 화면에 표시할 노선명을 정규화합니다.
    if (!route_display_label(data, route, route_label, sizeof(route_label))) {
        return 0;
    }

    // 응답 버퍼가 거의 차 있으면 추가하지 않습니다.
    if (strlen(response) + 700 >= response_size) {
        return 0;
    }

    // 두 번째 노선부터는 ';'로 구분합니다.
    if (item_index > 0) {
        str_append(response, response_size, ";");
    }

    // 출발지와 종점 이름을 찾습니다.
    start_name = find_node_name_by_id(data, route->start_stop_id);
    end_name = find_node_name_by_id(data, route->end_stop_id);
    // 노선 기본 정보를 먼저 응답에 붙입니다.
    if (!str_appendf(response,
                     response_size,
                     "%s:%s:%s:%s:",
                     route->route_no,
                     route_label,
                     start_name,
                     end_name)) {
        return 0;
    }
    // 노선에 포함된 정류장 이름을 순서대로 붙입니다.
    for (cur = route->stops; cur; cur = cur->next) {
        // 두 번째 정류장부터는 쉼표로 구분합니다.
        if (count > 0) {
            str_append(response, response_size, ",");
        }
        str_append(response, response_size, find_node_name_by_id(data, cur->stop_id));
        count++;
    }
    // 노선에 연결된 정류장 목록이 비어 있을 때는 '-'를 표시값으로 사용합니다.
    if (count == 0) {
        str_append(response, response_size, "-");
    }
    return 1;
}

// 버스 번호로 노선 정보를 찾습니다.
int handle_bus_search(AppData *data, const char *route_no, char *response, unsigned long response_size) {
    RouteNode *route;
    char route_keys[STOP_ROUTES_TEXT_SIZE] = "";
    int match_count = 0;

    // 검색어가 비어 있거나 통신 구분자를 포함하면 요청 오류로 처리합니다.
    if (!data || str_is_blank(route_no) || str_contains_protocol_delim(route_no)) {
        snprintf(response, response_size, "%s|%d:INVALID_REQUEST\n", PKT_ERROR, ERR_INVALID_REQUEST);
        return ERR_INVALID_REQUEST;
    }

    // 응답 종류와 성공 코드를 먼저 버퍼에 구성합니다.
    snprintf(response, response_size, "%s|%d|", PKT_BUS_SEARCH_RES, ERR_OK);
    // 정확한 일치부터 부분 일치까지 순서대로 찾습니다.
    for (int rank = 1; rank <= 3 && match_count < 10; ++rank) {
        // 전체 노선을 돌며 현재 순위에 맞는 노선을 찾습니다.
        for (route = data->routes; route; route = route->next) {
            if (route_match_rank(route, route_no) == rank) {
                // 이미 같은 노선으로 넣은 항목이면 건너뜁니다.
                if (!mark_bus_route_seen(data, route, route_keys, sizeof(route_keys))) {
                    continue;
                }
                // 최대 10개까지만 응답에 넣습니다.
                if (match_count >= 10 || !append_bus_route_item(data, route, response, response_size, match_count)) {
                    break;
                }
                match_count++;
            }
        }
    }

    // 검색 결과가 비어 있으면 결과 없음 코드로 응답을 마무리합니다.
    if (match_count == 0) {
        snprintf(response, response_size, "%s|%d|\n", PKT_BUS_SEARCH_RES, ERR_NOT_FOUND);
        return ERR_NOT_FOUND;
    }
    // 프로토콜 한 줄 응답을 완성하기 위해 마지막에 개행을 추가합니다.
    str_append(response, response_size, "\n");
    return ERR_OK;
}

// 지하철역 이름으로 역 정보를 찾습니다.
int handle_subway_search(AppData *data, const char *keyword, char *response, unsigned long response_size) {
    StationNode *cur;
    int count = 0;

    // 검색어가 비어 있거나 통신 구분자를 포함하면 요청 오류로 처리합니다.
    if (!data || str_is_blank(keyword) || str_contains_protocol_delim(keyword)) {
        snprintf(response, response_size, "%s|%d:INVALID_REQUEST\n", PKT_ERROR, ERR_INVALID_REQUEST);
        return ERR_INVALID_REQUEST;
    }

    // 응답 종류와 성공 코드를 먼저 버퍼에 구성합니다.
    snprintf(response, response_size, "%s|%d|", PKT_SUBWAY_SEARCH_RES, ERR_OK);
    // 모든 역을 돌며 이름, 노선, 환승 정보, 주소, ID를 검색합니다.
    for (cur = data->stations; cur; cur = cur->next) {
        if (keyword_match(cur->station_name, keyword) ||
            keyword_match(cur->line_no, keyword) ||
            keyword_match(cur->transfer_lines, keyword) ||
            keyword_match(cur->address, keyword) ||
            strcmp(cur->station_id, keyword) == 0) {
            // 응답 크기와 최대 개수를 넘기지 않습니다.
            if (strlen(response) + 300 >= response_size || count >= 20) {
                break;
            }
            // 두 번째 항목부터는 ';'로 구분합니다.
            if (count > 0) {
                str_append(response, response_size, ";");
            }
            // 역 ID, 역명, 노선, 환승 가능 노선을 응답에 붙입니다.
            str_appendf(response, response_size, "%s:%s:%s:%s",
                        cur->station_id,
                        cur->station_name,
                        cur->line_no,
                        cur->transfer_lines[0] ? cur->transfer_lines : "-");
            count++;
        }
    }
    // 검색 결과가 비어 있으면 결과 없음 코드로 응답을 마무리합니다.
    if (count == 0) {
        snprintf(response, response_size, "%s|%d|\n", PKT_SUBWAY_SEARCH_RES, ERR_NOT_FOUND);
        return ERR_NOT_FOUND;
    }
    // 프로토콜 한 줄 응답을 완성하기 위해 마지막에 개행을 추가합니다.
    str_append(response, response_size, "\n");
    return ERR_OK;
}
