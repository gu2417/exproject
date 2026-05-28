#ifndef TRANSIT_CLIENT_NET_H
#define TRANSIT_CLIENT_NET_H

#include "net_compat.h"

// 서버 접속과 요청/응답 송수신 함수들입니다.
int client_connect(socket_t *out_fd, const char *host, int port);
int client_send_line(socket_t fd, const char *line);
int client_recv_line(socket_t fd, char *buf, unsigned long buf_size);
int client_send_request(socket_t fd, const char *request);

#endif
