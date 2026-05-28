#ifndef TRANSIT_CLIENT_HANDLER_H
#define TRANSIT_CLIENT_HANDLER_H

#include "net_compat.h"
#include "types.h"

// 단일 클라이언트 연결의 수신, 라우팅, 응답 전송을 담당합니다.
void handle_client(socket_t client_fd, const char *client_ip, AppData *data);

#endif
