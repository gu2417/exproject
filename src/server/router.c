#include "router.h"

#include "facility_service.h"
#include "operation_service.h"
#include "protocol.h"
#include "search_service.h"
#include "string_utils.h"
#include "transfer_service.h"
#include "user_state_service.h"

#include <stdio.h>
#include <string.h>

typedef int (*TransitService)(AppData *, const char *, char *, unsigned long);

// 최근 검색 파일에 저장할 화면용 검색 문장을 구성합니다.
static void make_recent_pair_text(const char *request_body, const char *sep_text, char *out, size_t out_size) {
    char buf[MAX_FIELD_LEN * 2];
    char *right;

    // 최근 검색 문장을 담을 버퍼가 준비된 경우에만 기록 문자열을 생성합니다.
    if (!out || out_size == 0) {
        return;
    }
    // 기본 결과는 빈 문자열로 둡니다.
    out[0] = '\0';
    // 요청 본문이 비어 있으면 최근 검색 기록용 문장을 생성하지 않습니다.
    if (!request_body) {
        return;
    }

    // 요청 본문을 분리하기 전에 작업용 버퍼로 복사합니다.
    safe_strcpy(buf, sizeof(buf), request_body);
    // 출발지:도착지처럼 두 값 사이의 구분자를 찾습니다.
    right = strchr(buf, ITEM_SEP);
    if (!right) {
        safe_strcpy(out, out_size, request_body);
        return;
    }
    // 왼쪽 값과 오른쪽 값을 나눕니다.
    *right++ = '\0';
    // 앞뒤 빈칸을 제거합니다.
    str_trim(buf);
    str_trim(right);
    // 두 값을 보기 좋은 문장으로 합칩니다.
    snprintf(out, out_size, "%s%s%s", buf, sep_text, right);
}

// 검색이 성공했을 때 최근 검색 기록에 저장합니다.
static void save_recent_search(int result, const char *recent_type, const char *request_body) {
    char keyword[MAX_FIELD_LEN * 2];

    // 성공한 검색만 최근 검색 기록에 남깁니다.
    if (result != ERR_OK || str_is_blank(recent_type) || str_is_blank(request_body)) {
        return;
    }

    // 길찾기와 편의시설 검색은 두 입력값을 조합해 기록용 문장으로 정리합니다.
    if (strcmp(recent_type, "route") == 0) {
        make_recent_pair_text(request_body, "->", keyword, sizeof(keyword));
    } else if (strcmp(recent_type, "facility") == 0) {
        make_recent_pair_text(request_body, "/", keyword, sizeof(keyword));
    } else {
        // 일반 검색은 검색어 그대로 기록합니다.
        safe_strcpy(keyword, sizeof(keyword), request_body);
        str_trim(keyword);
    }

    // 서버 쪽 최근 검색 파일에 저장합니다.
    server_recent_add(recent_type, keyword);
}

// 검색 요청을 처리하고 필요하면 기록에 남깁니다.
static int run_service_and_save_recent(TransitService service,
                                       AppData *data,
                                       const char *request_body,
                                       char *response,
                                       unsigned long response_size,
                                       const char *recent_type) {
    // 실제 기능 처리 함수를 먼저 실행합니다.
    int result = service(data, request_body, response, response_size);
    // 처리 결과가 성공이면 최근 검색 기록을 남깁니다.
    save_recent_search(result, recent_type, request_body);
    return result;
}

// 요청 종류를 보고 알맞은 처리 함수로 넘깁니다.
int route_request(AppData *data, const char *request, char *response, unsigned long response_size) {
    char packet[MAX_PACKET_SIZE];
    char *type;
    char *request_body;
    // 요청 문자열과 응답 버퍼가 유효하지 않은 경우 즉시 오류 응답을 작성합니다.
    if (!request || !response || response_size == 0) {
        return ERR_INVALID_REQUEST;
    }

    // 원본 요청을 보존하기 위해 임시 버퍼에 복사합니다.
    safe_strcpy(packet, sizeof(packet), request);
    // 응답 파싱 전에 문자열 끝의 개행 문자를 제거합니다.
    strip_newline(packet);

    // 요청 종류는 맨 앞에서 시작합니다.
    type = packet;
    // 파일에서 온 문자열처럼 BOM이 붙어 있으면 건너뜁니다.
    if ((unsigned char)type[0] == 0xEF &&
        (unsigned char)type[1] == 0xBB &&
        (unsigned char)type[2] == 0xBF) {
        type += 3;
    }
    // '|' 뒤쪽은 요청 내용입니다.
    request_body = strchr(packet, FIELD_SEP);
    if (request_body) {
        // 요청 종류와 내용을 나눕니다.
        *request_body = '\0';
        request_body++;
    } else {
        // 내용이 없는 요청은 빈 문자열로 둡니다.
        request_body = "";
    }

    // 최초 접속 확인 요청에 응답합니다.
    if (strcmp(type, PKT_HELLO) == 0) {
        snprintf(response, response_size, "%s|OK:%s\n", PKT_HELLO_RES, SERVER_VERSION);
        return ERR_OK;
    }
    // 연결 확인 요청에 응답합니다.
    if (strcmp(type, PKT_PING) == 0) {
        snprintf(response, response_size, "%s|\n", PKT_PONG);
        return ERR_OK;
    }
    // 정류장 검색 패킷을 정류장 조회 서비스로 전달합니다.
    if (strcmp(type, PKT_STOP_SEARCH_REQ) == 0) {
        return run_service_and_save_recent(handle_stop_search, data, request_body, response, response_size, "stop");
    }
    // 버스 검색 패킷을 노선 조회 서비스로 전달합니다.
    if (strcmp(type, PKT_BUS_SEARCH_REQ) == 0) {
        return run_service_and_save_recent(handle_bus_search, data, request_body, response, response_size, "bus");
    }
    // 지하철역 검색 패킷을 역 조회 서비스로 전달합니다.
    if (strcmp(type, PKT_SUBWAY_SEARCH_REQ) == 0) {
        return run_service_and_save_recent(handle_subway_search, data, request_body, response, response_size, "station");
    }
    // 출발지·도착지 기반 최단 경로 요청을 처리합니다.
    if (strcmp(type, PKT_TRANSFER_REQ) == 0) {
        return run_service_and_save_recent(handle_transfer, data, request_body, response, response_size, "route");
    }
    // 편의시설 조회 패킷을 시설 검색 서비스로 전달합니다.
    if (strcmp(type, PKT_FACILITY_REQ) == 0) {
        return run_service_and_save_recent(handle_facility, data, request_body, response, response_size, "facility");
    }
    // 도착 예정 시간 패킷을 운행 정보 서비스로 전달합니다.
    if (strcmp(type, PKT_ARRIVAL_REQ) == 0) {
        return handle_arrival(data, request_body, response, response_size);
    }
    // 혼잡도 패킷을 운행 정보 서비스로 전달합니다.
    if (strcmp(type, PKT_CROWDING_REQ) == 0) {
        return handle_crowding(data, request_body, response, response_size);
    }
    // 즐겨찾기 추가 패킷을 사용자 저장 서비스로 전달합니다.
    if (strcmp(type, PKT_FAVORITE_ADD_REQ) == 0) {
        return handle_favorite_add(data, request_body, response, response_size);
    }
    // 즐겨찾기 조회 패킷을 사용자 저장 서비스로 전달합니다.
    if (strcmp(type, PKT_FAVORITE_LIST_REQ) == 0) {
        return handle_favorite_list(response, response_size);
    }
    // 즐겨찾기 삭제 패킷을 사용자 저장 서비스로 전달합니다.
    if (strcmp(type, PKT_FAVORITE_DELETE_REQ) == 0) {
        return handle_favorite_delete(request_body, response, response_size);
    }
    // 최근 검색 기록 저장과 조회 요청을 처리합니다.
    if (strcmp(type, PKT_RECENT_LIST_REQ) == 0) {
        return handle_recent_list(response, response_size);
    }

    // 등록된 패킷 종류와 일치하지 않으면 잘못된 요청 응답을 작성합니다.
    snprintf(response, response_size, "%s|%d:INVALID_REQUEST\n", PKT_ERROR, ERR_INVALID_REQUEST);
    return ERR_INVALID_REQUEST;
}
