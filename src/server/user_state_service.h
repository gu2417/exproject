#ifndef TRANSIT_USER_STATE_SERVICE_H
#define TRANSIT_USER_STATE_SERVICE_H

#include "types.h"

// 즐겨찾기 추가, 조회, 삭제 요청을 처리합니다.
int handle_favorite_add(AppData *data, const char *payload, char *response, unsigned long response_size);
int handle_favorite_list(char *response, unsigned long response_size);
int handle_favorite_delete(const char *payload, char *response, unsigned long response_size);
// 최근 검색 기록 저장과 조회 요청을 처리합니다.
int handle_recent_list(char *response, unsigned long response_size);
void server_recent_add(const char *type, const char *keyword);

#endif
