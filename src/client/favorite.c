#include "favorite.h"

#include "client_net.h"
#include "protocol.h"
#include "string_utils.h"

#include <stdio.h>
#include <string.h>

static int contains_request_sep(const char *value) {
    // 요청 구분자가 들어 있으면 서버에서 잘못 해석될 수 있습니다.
    return value && (str_contains_protocol_delim(value) || strchr(value, ITEM_SEP));
}

// 즐겨찾기 추가에 필요한 요청 패킷을 서버로 전송합니다.
int favorite_add(socket_t fd, const char *type, const char *id, const char *name) {
    char request[MAX_PACKET_SIZE];

    // 종류, 값, 이름 중 하나라도 비면 추가하지 않습니다.
    if (str_is_blank(type) || str_is_blank(id) || str_is_blank(name) ||
        contains_request_sep(type) || contains_request_sep(id) || contains_request_sep(name)) {
        printf("입력값을 다시 확인해주세요.\n");
        return 0;
    }

    // 종류, 값, 표시 이름을 프로토콜 형식의 추가 요청으로 조립합니다.
    snprintf(request, sizeof(request), "%s|%s:%s:%s\n", PKT_FAVORITE_ADD_REQ, type, id, name);
    // 완성된 요청 문자열을 서버로 전송합니다.
    return client_send_request(fd, request);
}

// 저장된 즐겨찾기 목록을 요청합니다.
void favorite_list(socket_t fd) {
    char request[MAX_PACKET_SIZE];

    // 저장된 즐겨찾기 목록을 조회하는 요청 패킷을 구성합니다.
    snprintf(request, sizeof(request), "%s|\n", PKT_FAVORITE_LIST_REQ);
    // 완성된 요청 문자열을 서버로 전송합니다.
    client_send_request(fd, request);
}

// 선택한 즐겨찾기 삭제를 요청합니다.
int favorite_delete(socket_t fd, int index) {
    char request[MAX_PACKET_SIZE];

    // 삭제 번호는 1번부터 사용하므로 0 이하 값은 요청하지 않습니다.
    if (index <= 0) {
        printf("삭제할 번호를 다시 확인해주세요.\n");
        return 0;
    }

    // 삭제할 목록 번호를 프로토콜 형식의 삭제 요청으로 조립합니다.
    snprintf(request, sizeof(request), "%s|%d\n", PKT_FAVORITE_DELETE_REQ, index);
    // 완성된 요청 문자열을 서버로 전송합니다.
    return client_send_request(fd, request);
}
