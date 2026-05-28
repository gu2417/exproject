#include "recent_search.h"

#include "client_net.h"
#include "protocol.h"

#include <stdio.h>

// 최근 검색 기록을 요청합니다.
void recent_list(socket_t fd) {
    char request[MAX_PACKET_SIZE];

    // 최근 검색 기록 조회용 요청 패킷을 구성합니다.
    snprintf(request, sizeof(request), "%s|\n", PKT_RECENT_LIST_REQ);
    // 완성된 요청 문자열을 서버로 전송합니다.
    client_send_request(fd, request);
}
