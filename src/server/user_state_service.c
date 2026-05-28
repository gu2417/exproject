#include "user_state_service.h"

#include "file_utils.h"
#include "linked_list.h"
#include "log.h"
#include "protocol.h"
#include "string_utils.h"

#include <stdio.h>
#include <string.h>

#define SERVER_FAVORITE_PATH "data/favorites.txt"
#define SERVER_FAVORITE_TEMP_PATH "data/favorites.tmp"
#define SERVER_RECENT_PATH "data/recent_search.txt"
#define SERVER_RECENT_TEMP_PATH "data/recent_search.tmp"
#define SERVER_RECENT_MAX_COUNT 20
#define UTF8_BUN "\xEB\xB2\x88"

// 저장된 종류 값을 화면에 보일 한글 이름으로 바꿉니다.
static const char *favorite_type_label(const char *type) {
    // 저장된 종류 값이 NULL이면 화면 표시값을 빈 문자열로 반환합니다.
    if (!type) {
        return "";
    }
    // 정류장, 버스, 지하철역 값을 각각 한글로 바꿉니다.
    if (strcmp(type, "stop") == 0) {
        return "정류장";
    }
    if (strcmp(type, "bus") == 0) {
        return "버스";
    }
    if (strcmp(type, "station") == 0) {
        return "지하철역";
    }
    return type;
}

// 저장 값에 통신 구분자가 들어 있는지 확인합니다.
static int contains_packet_field_delim(const char *value) {
    // 구분자가 있으면 파일이나 응답 형식이 깨질 수 있습니다.
    return value && (str_contains_protocol_delim(value) || strchr(value, ITEM_SEP));
}

// 사용자가 입력한 즐겨찾기 종류를 내부 값으로 바꿉니다.
static int normalize_favorite_type(const char *input, char *out, size_t out_size) {
    char buf[64];

    // 입력값과 저장 버퍼가 모두 유효해야 종류 변환을 진행합니다.
    if (str_is_blank(input) || !out || out_size == 0) {
        return 0;
    }

    // 원본 입력을 임시 버퍼에 복사해서 정리합니다.
    safe_strcpy(buf, sizeof(buf), input);
    str_trim(buf);

    // 정류장 입력이면 stop으로 저장합니다.
    if (strcmp(buf, "1") == 0 || strcmp(buf, "정류장") == 0 || strcmp(buf, "버스정류장") == 0 ||
        strcmp(buf, "stop") == 0) {
        safe_strcpy(out, out_size, "stop");
        return 1;
    }
    // 버스 입력이면 bus로 저장합니다.
    if (strcmp(buf, "2") == 0 || strcmp(buf, "버스") == 0 || strcmp(buf, "버스번호") == 0 ||
        strcmp(buf, "노선") == 0 || strcmp(buf, "bus") == 0) {
        safe_strcpy(out, out_size, "bus");
        return 1;
    }
    // 지하철역 입력이면 station으로 저장합니다.
    if (strcmp(buf, "3") == 0 || strcmp(buf, "지하철역") == 0 || strcmp(buf, "역") == 0 ||
        strcmp(buf, "station") == 0) {
        safe_strcpy(out, out_size, "station");
        return 1;
    }
    return 0;
}

// 같은 즐겨찾기가 이미 있는지 확인합니다.
static int favorite_exists(const char *type, const char *id) {
    FILE *fp;
    char line[1024];

    // 즐겨찾기 파일을 읽기 모드로 엽니다.
    fp = fopen(SERVER_FAVORITE_PATH, "r");
    if (!fp) {
        return 0;
    }

    // 파일의 각 줄을 읽어 type과 id를 비교합니다.
    while (fgets(line, sizeof(line), fp)) {
        char *p1;
        char *p2;

        // 줄바꿈을 지우고 type|id|name 형식으로 나눕니다.
        strip_newline(line);
        p1 = strchr(line, '|');
        if (!p1) {
            continue;
        }
        *p1++ = '\0';
        p2 = strchr(p1, '|');
        if (!p2) {
            continue;
        }
        *p2 = '\0';

        // 종류와 ID가 같으면 이미 저장된 항목입니다.
        if (strcmp(line, type) == 0 && strcmp(p1, id) == 0) {
            fclose(fp);
            return 1;
        }
    }

    fclose(fp);
    return 0;
}

// 입력한 값이 ID나 이름과 같은지 확인합니다.
static int text_matches_input(const char *value, const char *id, const char *name) {
    return !str_is_blank(value) &&
           ((!str_is_blank(id) && strcmp(value, id) == 0) ||
            (!str_is_blank(name) && strcmp(value, name) == 0));
}

// 버스 노선명과 입력값이 같은 노선으로 볼 수 있는지 확인합니다.
static int route_name_matches_value(const char *route_name, const char *value) {
    size_t route_len;
    size_t value_len;
    size_t bun_len = strlen(UTF8_BUN);

    // 노선명 비교 값이 비어 있으면 매칭 대상에서 제외합니다.
    if (str_is_blank(route_name) || str_is_blank(value)) {
        return 0;
    }
    // 완전히 같으면 바로 성공입니다.
    if (strcmp(route_name, value) == 0) {
        return 1;
    }

    route_len = strlen(route_name);
    value_len = strlen(value);

    // route_name 끝의 '번'을 빼고 입력값과 비교합니다.
    if (route_len > bun_len &&
        strcmp(route_name + route_len - bun_len, UTF8_BUN) == 0 &&
        route_len - bun_len == value_len &&
        strncmp(route_name, value, value_len) == 0) {
        return 1;
    }
    // 입력값 끝의 '번'을 빼고 노선명과 비교합니다.
    if (value_len > bun_len &&
        strcmp(value + value_len - bun_len, UTF8_BUN) == 0 &&
        value_len - bun_len == route_len &&
        strncmp(value, route_name, route_len) == 0) {
        return 1;
    }
    // 입력값이 노선명 앞부분과 맞는지도 확인합니다.
    if (route_len > value_len && strncmp(route_name, value, value_len) == 0) {
        const char *tail = route_name + value_len;
        if (*tail == '\0' || *tail == ' ' || *tail == '-' || *tail == '(' ||
            strncmp(tail, UTF8_BUN, bun_len) == 0) {
            return 1;
        }
    }
    return 0;
}

// 즐겨찾기 입력값이 버스 노선과 맞는지 확인합니다.
static int route_matches_input(const RouteNode *route, const char *id, const char *name) {
    // 노선 정보가 전달된 경우에만 즐겨찾기 입력값과 비교합니다.
    if (!route) {
        return 0;
    }
    // 노선 번호, 노선명, '번' 포함 여부를 모두 확인합니다.
    return text_matches_input(route->route_no, id, name) ||
           text_matches_input(route->route_name, id, name) ||
           route_name_matches_value(route->route_name, id) ||
           route_name_matches_value(route->route_name, name) ||
           route_name_matches_value(route->route_no, id) ||
           route_name_matches_value(route->route_no, name);
}

// 실제 데이터에 있는 항목인지 확인합니다.
static int favorite_target_exists(const AppData *data, const char *type, const char *id, const char *name) {
    // 즐겨찾기 종류와 실제 데이터가 준비된 경우에만 유효성 검사를 수행합니다.
    if (!data || str_is_blank(type)) {
        return 0;
    }

    // 정류장은 ID와 이름을 모두 확인합니다.
    if (strcmp(type, "stop") == 0) {
        const StopNode *cur;
        if (find_stop_by_id(data, id) || find_stop_by_id(data, name)) {
            return 1;
        }
        // 이름으로도 한 번 더 확인합니다.
        for (cur = data->stops; cur; cur = cur->next) {
            if (text_matches_input(cur->stop_name, id, name)) {
                return 1;
            }
        }
        return 0;
    }

    // 버스는 노선 번호와 노선명을 모두 확인합니다.
    if (strcmp(type, "bus") == 0) {
        const RouteNode *cur;
        if (find_route_by_no(data, id) || find_route_by_no(data, name)) {
            return 1;
        }
        // 노선명에 '번'이 붙은 경우도 확인합니다.
        for (cur = data->routes; cur; cur = cur->next) {
            if (route_matches_input(cur, id, name)) {
                return 1;
            }
        }
        return 0;
    }

    // 지하철역은 역 ID와 역명을 확인합니다.
    if (strcmp(type, "station") == 0) {
        const StationNode *cur;
        if (find_station_by_id(data, id) || find_station_by_id(data, name)) {
            return 1;
        }
        // 역명으로도 한 번 더 확인합니다.
        for (cur = data->stations; cur; cur = cur->next) {
            if (text_matches_input(cur->station_name, id, name)) {
                return 1;
            }
        }
        return 0;
    }

    return 0;
}

// 잘못된 즐겨찾기 대상이면 로그에 남깁니다.
static void log_invalid_favorite_target(const char *type, const char *id, const char *name) {
    char message[512];

    // 잘못된 즐겨찾기 입력을 기록할 로그 메시지를 구성합니다.
    snprintf(message,
             sizeof(message),
             "favorite target not found in loaded CSV data: type=%s id=%s name=%s",
             type ? type : "",
             id ? id : "",
             name ? name : "");
    // 오류 로그로 기록합니다.
    log_error("-", "FAVORITE_ADD", ERR_NOT_FOUND, message);
}

// 즐겨찾기를 파일에 저장합니다.
int handle_favorite_add(AppData *data, const char *payload, char *response, unsigned long response_size) {
    FILE *fp;
    char type_input[64];
    char type[64];
    char id[128];
    char name[256];

    // 요청 내용은 종류:값:표시이름 형식이어야 합니다.
    if (!split_three_fields(payload, ITEM_SEP, type_input, sizeof(type_input), id, sizeof(id), name, sizeof(name)) ||
        !normalize_favorite_type(type_input, type, sizeof(type)) ||
        str_is_blank(id) ||
        str_is_blank(name) ||
        contains_packet_field_delim(id) ||
        contains_packet_field_delim(name)) {
        snprintf(response, response_size, "%s|%d:INVALID_REQUEST\n", PKT_ERROR, ERR_INVALID_REQUEST);
        return ERR_INVALID_REQUEST;
    }

    // 실제 CSV 데이터에 없는 항목이면 저장하지 않습니다.
    if (!favorite_target_exists(data, type, id, name)) {
        log_invalid_favorite_target(type, id, name);
        snprintf(response,
                 response_size,
                 "%s|%d|CSV 데이터에 없는 항목이라 즐겨찾기에 저장하지 않았습니다.\n",
                 PKT_FAVORITE_ADD_RES,
                 ERR_NOT_FOUND);
        return ERR_NOT_FOUND;
    }

    // 즐겨찾기 파일이 들어갈 폴더를 준비합니다.
    ensure_dir("data");
    // 이미 같은 항목이 있으면 중복 저장하지 않습니다.
    if (favorite_exists(type, id)) {
        snprintf(response, response_size, "%s|%d|이미 저장된 즐겨찾기입니다.\n", PKT_FAVORITE_ADD_RES, ERR_OK);
        return ERR_OK;
    }

    // 즐겨찾기 파일을 추가 모드로 엽니다.
    fp = fopen(SERVER_FAVORITE_PATH, "a");
    if (!fp) {
        snprintf(response, response_size, "%s|%d|서버 즐겨찾기 파일을 열 수 없습니다.\n", PKT_FAVORITE_ADD_RES, ERR_SERVER_ERROR);
        return ERR_SERVER_ERROR;
    }

    // type|id|name 형식으로 한 줄 저장합니다.
    fprintf(fp, "%s|%s|%s\n", type, id, name);
    fclose(fp);
    // 즐겨찾기 저장 성공 여부를 사용자에게 보여줄 응답 문구로 구성합니다.
    snprintf(response,
             response_size,
             "%s|%d|즐겨찾기를 서버에 저장했습니다: %s | %s | %s\n",
             PKT_FAVORITE_ADD_RES,
             ERR_OK,
             favorite_type_label(type),
             id,
             name);
    return ERR_OK;
}

// 사용자 저장 파일에서 즐겨찾기 목록을 읽어 응답으로 변환합니다.
int handle_favorite_list(char *response, unsigned long response_size) {
    FILE *fp;
    char line[1024];
    int count = 0;

    // 사용자 저장 폴더를 확인한 뒤 목록 응답의 기본 형식을 구성합니다.
    ensure_dir("data");
    snprintf(response, response_size, "%s|%d|", PKT_FAVORITE_LIST_RES, ERR_OK);

    // 즐겨찾기 저장 파일이 아직 생성되지 않은 경우 빈 목록을 정상 응답으로 반환합니다.
    fp = fopen(SERVER_FAVORITE_PATH, "r");
    if (!fp) {
        str_append(response, response_size, "\n");
        return ERR_OK;
    }

    // 즐겨찾기 파일을 순회하며 각 줄을 목록 항목으로 해석합니다.
    while (fgets(line, sizeof(line), fp)) {
        char *type;
        char *id;
        char *name;
        size_t needed;

        // type|id|name 형식으로 나눕니다.
        strip_newline(line);
        type = line;
        id = strchr(type, '|');
        if (!id) {
            continue;
        }
        *id++ = '\0';
        name = strchr(id, '|');
        if (!name) {
            continue;
        }
        *name++ = '\0';
        // 필수 필드 중 빈 값이 있는 기록은 응답 목록에서 제외합니다.
        if (str_is_blank(type) || str_is_blank(id) || str_is_blank(name)) {
            continue;
        }

        // 응답 버퍼가 넘치지 않는지 확인합니다.
        needed = strlen(type) + strlen(id) + strlen(name) + 4;
        if (strlen(response) + needed + 2 >= response_size) {
            break;
        }
        // 두 번째 항목부터는 ';'로 구분합니다.
        if (count > 0) {
            str_append(response, response_size, ";");
        }
        // 클라이언트가 읽을 수 있게 ':' 형식으로 붙입니다.
        str_appendf(response, response_size, "%s:%s:%s", type, id, name);
        count++;
    }

    // 파일을 닫고 응답 끝에 줄바꿈을 붙입니다.
    fclose(fp);
    str_append(response, response_size, "\n");
    return ERR_OK;
}

// 번호로 즐겨찾기를 삭제합니다.
int handle_favorite_delete(const char *payload, char *response, unsigned long response_size) {
    FILE *src;
    FILE *dst;
    char line[1024];
    char deleted_line[1024] = "";
    int target_index;
    int current = 1;
    int deleted = 0;

    // 삭제할 번호를 정수로 바꿉니다.
    if (!str_to_int(payload, &target_index) || target_index <= 0) {
        snprintf(response, response_size, "%s|%d:INVALID_REQUEST\n", PKT_ERROR, ERR_INVALID_REQUEST);
        return ERR_INVALID_REQUEST;
    }

    // 사용자별 저장 파일을 쓰기 위한 폴더를 확인합니다.
    ensure_dir("data");
    // 원본 즐겨찾기 파일을 엽니다.
    src = fopen(SERVER_FAVORITE_PATH, "r");
    if (!src) {
        snprintf(response, response_size, "%s|%d|저장된 즐겨찾기가 없습니다.\n", PKT_FAVORITE_DELETE_RES, ERR_OK);
        return ERR_OK;
    }
    // 삭제 결과를 임시 파일에 다시 씁니다.
    dst = fopen(SERVER_FAVORITE_TEMP_PATH, "w");
    if (!dst) {
        fclose(src);
        snprintf(response, response_size, "%s|%d|서버 즐겨찾기 파일을 수정할 수 없습니다.\n", PKT_FAVORITE_DELETE_RES, ERR_SERVER_ERROR);
        return ERR_SERVER_ERROR;
    }

    // 원본 즐겨찾기 파일을 순회하며 삭제 대상 번호를 찾습니다.
    while (fgets(line, sizeof(line), src)) {
        char original[1024];

        // 원본 줄은 다시 파일에 쓸 수 있게 보관합니다.
        safe_strcpy(original, sizeof(original), line);
        strip_newline(line);
        // 내용이 없는 줄은 저장 기록으로 보지 않고 넘어갑니다.
        if (str_is_blank(line)) {
            continue;
        }

        // 사용자가 고른 번호면 삭제 대상으로 저장합니다.
        if (current == target_index) {
            safe_strcpy(deleted_line, sizeof(deleted_line), line);
            deleted = 1;
        } else {
            // 삭제 대상이 아니면 임시 파일에 그대로 씁니다.
            fputs(original, dst);
            if (original[strlen(original) - 1] != '\n') {
                fputc('\n', dst);
            }
        }
        // 다음 목록 번호로 넘어갑니다.
        current++;
    }

    // 원본과 임시 파일을 닫습니다.
    fclose(src);
    fclose(dst);

    // 삭제할 번호를 찾지 못했으면 임시 파일을 지웁니다.
    if (!deleted) {
        remove(SERVER_FAVORITE_TEMP_PATH);
        snprintf(response, response_size, "%s|%d|해당 번호의 즐겨찾기가 없습니다.\n", PKT_FAVORITE_DELETE_RES, ERR_OK);
        return ERR_OK;
    }

    // 원본 파일을 임시 파일로 교체합니다.
    if (remove(SERVER_FAVORITE_PATH) != 0 || rename(SERVER_FAVORITE_TEMP_PATH, SERVER_FAVORITE_PATH) != 0) {
        remove(SERVER_FAVORITE_TEMP_PATH);
        snprintf(response, response_size, "%s|%d|즐겨찾기 삭제 중 서버 파일 처리에 실패했습니다.\n", PKT_FAVORITE_DELETE_RES, ERR_SERVER_ERROR);
        return ERR_SERVER_ERROR;
    }

    // 삭제된 항목 정보가 있으면 사용자 안내 문구에 함께 반영합니다.
    if (deleted_line[0]) {
        char *type = deleted_line;
        char *id = strchr(type, '|');
        char *name = NULL;

        // 삭제한 줄을 type, id, name으로 나눕니다.
        if (id) {
            *id++ = '\0';
            name = strchr(id, '|');
            if (name) {
                *name++ = '\0';
            }
        }
        snprintf(response,
                 response_size,
                 "%s|%d|즐겨찾기를 서버에서 삭제했습니다: %s | %s | %s\n",
                 PKT_FAVORITE_DELETE_RES,
                 ERR_OK,
                 favorite_type_label(type),
                 id ? id : "",
                 name ? name : "");
    } else {
        // 삭제된 항목 정보를 해석하지 못하면 기본 삭제 완료 문구를 반환합니다.
        snprintf(response, response_size, "%s|%d|즐겨찾기를 서버에서 삭제했습니다.\n", PKT_FAVORITE_DELETE_RES, ERR_OK);
    }
    return ERR_OK;
}

// 최근 검색 기록을 파일에 남깁니다.
void server_recent_add(const char *type, const char *keyword) {
    FILE *fp;
    char recent[SERVER_RECENT_MAX_COUNT - 1][1024];
    char line[1024];
    char record[1024];
    char ts[32];
    int count = 0;
    int i;

    // 기록할 값이 비었거나 구분자가 있으면 저장하지 않습니다.
    if (str_is_blank(type) || str_is_blank(keyword) ||
        contains_packet_field_delim(type) ||
        contains_packet_field_delim(keyword)) {
        return;
    }

    // 사용자별 저장 파일을 쓰기 위한 폴더를 확인합니다.
    ensure_dir("data");

    // 기존 최근 검색 파일을 열어 현재 저장된 기록을 불러옵니다.
    fp = fopen(SERVER_RECENT_PATH, "r");
    if (fp) {
        // 기존 기록을 배열에 보관합니다.
        while (fgets(line, sizeof(line), fp)) {
            strip_newline(line);
            // 내용이 없는 줄은 저장 기록으로 보지 않고 넘어갑니다.
            if (str_is_blank(line)) {
                continue;
            }
            // 최대 개수를 넘으면 가장 오래된 기록을 밀어냅니다.
            if (count >= SERVER_RECENT_MAX_COUNT - 1) {
                for (i = 1; i < SERVER_RECENT_MAX_COUNT - 1; i++) {
                    safe_strcpy(recent[i - 1], sizeof(recent[i - 1]), recent[i]);
                }
                count = SERVER_RECENT_MAX_COUNT - 2;
            }
            // 읽은 기록을 배열에 저장합니다.
            safe_strcpy(recent[count], sizeof(recent[count]), line);
            count++;
        }
        fclose(fp);
    }

    // 현재 시각과 검색 내용을 묶어 새 최근 검색 레코드를 구성합니다.
    current_timestamp(ts, sizeof(ts));
    snprintf(record, sizeof(record), "%s|%s|%s", type, keyword, ts);

    // 임시 파일에 기존 기록과 새 기록을 다시 씁니다.
    fp = fopen(SERVER_RECENT_TEMP_PATH, "w");
    if (!fp) {
        return;
    }
    // 기존 기록을 먼저 씁니다.
    for (i = 0; i < count; i++) {
        fprintf(fp, "%s\n", recent[i]);
    }
    // 새 기록을 마지막에 씁니다.
    fprintf(fp, "%s\n", record);
    fclose(fp);

    // 기존 파일을 새 임시 파일로 교체합니다.
    remove(SERVER_RECENT_PATH);
    if (rename(SERVER_RECENT_TEMP_PATH, SERVER_RECENT_PATH) != 0) {
        remove(SERVER_RECENT_TEMP_PATH);
    }
}

// 저장된 최근 검색 기록을 최신 항목부터 응답으로 구성합니다.
int handle_recent_list(char *response, unsigned long response_size) {
    FILE *fp;
    char line[1024];
    char recent[SERVER_RECENT_MAX_COUNT][1024];
    int count = 0;
    int output_count = 0;
    int i;

    // 사용자 저장 폴더를 확인한 뒤 목록 응답의 기본 형식을 구성합니다.
    ensure_dir("data");
    snprintf(response, response_size, "%s|%d|", PKT_RECENT_LIST_RES, ERR_OK);

    // 최근 검색 파일이 없을 때는 빈 기록 목록을 정상 응답으로 반환합니다.
    fp = fopen(SERVER_RECENT_PATH, "r");
    if (!fp) {
        str_append(response, response_size, "\n");
        return ERR_OK;
    }

    // 최근 검색 파일을 순회하며 유효한 기록만 배열에 적재합니다.
    while (fgets(line, sizeof(line), fp)) {
        char parsed[1024];
        char *type;
        char *keyword;
        char *ts;

        // 줄바꿈과 BOM을 정리합니다.
        strip_newline(line);
        if ((unsigned char)line[0] == 0xEF &&
            (unsigned char)line[1] == 0xBB &&
            (unsigned char)line[2] == 0xBF) {
            memmove(line, line + 3, strlen(line + 3) + 1);
        }
        // type|keyword|time 형식인지 확인하기 위해 복사합니다.
        safe_strcpy(parsed, sizeof(parsed), line);
        type = parsed;
        keyword = strchr(type, '|');
        if (!keyword) {
            continue;
        }
        *keyword++ = '\0';
        ts = strchr(keyword, '|');
        if (!ts) {
            continue;
        }
        *ts++ = '\0';
        // 빈 칸이 있으면 건너뜁니다.
        if (str_is_blank(type) || str_is_blank(keyword) || str_is_blank(ts)) {
            continue;
        }

        // 최대 개수를 넘으면 오래된 기록을 밀어냅니다.
        if (count >= SERVER_RECENT_MAX_COUNT) {
            for (i = 1; i < SERVER_RECENT_MAX_COUNT; i++) {
                safe_strcpy(recent[i - 1], sizeof(recent[i - 1]), recent[i]);
            }
            count = SERVER_RECENT_MAX_COUNT - 1;
        }
        // 확인된 기록을 배열에 저장합니다.
        safe_strcpy(recent[count], sizeof(recent[count]), line);
        count++;
    }

    // 파일 읽기를 끝냅니다.
    fclose(fp);

    // 최신 기록부터 거꾸로 응답에 붙입니다.
    for (i = count - 1; i >= 0; i--) {
        char parsed[1024];
        char *type;
        char *keyword;
        char *ts;
        size_t needed;

        // 저장된 한 줄을 다시 type, keyword, time으로 나눕니다.
        safe_strcpy(parsed, sizeof(parsed), recent[i]);
        type = parsed;
        keyword = strchr(type, '|');
        if (!keyword) {
            continue;
        }
        *keyword++ = '\0';
        ts = strchr(keyword, '|');
        if (!ts) {
            continue;
        }
        *ts++ = '\0';

        // 응답 버퍼가 넘치지 않는지 확인합니다.
        needed = strlen(type) + strlen(keyword) + strlen(ts) + 4;
        if (strlen(response) + needed + 2 >= response_size) {
            break;
        }
        // 두 번째 항목부터는 ';'로 구분합니다.
        if (output_count > 0) {
            str_append(response, response_size, ";");
        }
        // 클라이언트가 읽을 수 있는 형식으로 붙입니다.
        str_appendf(response, response_size, "%s:%s:%s", type, keyword, ts);
        output_count++;
    }

    // 프로토콜 한 줄 응답을 완성하기 위해 마지막에 개행을 추가합니다.
    str_append(response, response_size, "\n");
    return ERR_OK;
}
