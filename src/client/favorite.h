#ifndef TRANSIT_FAVORITE_H
#define TRANSIT_FAVORITE_H

#include "net_compat.h"

// 즐겨찾기 관련 요청 패킷 생성과 서버 전송을 담당합니다.
int favorite_add(socket_t fd, const char *type, const char *id, const char *name);
void favorite_list(socket_t fd);
int favorite_delete(socket_t fd, int index);

#endif
