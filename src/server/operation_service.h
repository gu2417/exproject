#ifndef TRANSIT_OPERATION_SERVICE_H
#define TRANSIT_OPERATION_SERVICE_H

#include "types.h"

// 운행 정보 조회 요청을 받아 도착 예정 시간과 혼잡도 응답을 생성합니다.
int handle_arrival(AppData *data, const char *payload, char *response, unsigned long response_size);
int handle_crowding(AppData *data, const char *payload, char *response, unsigned long response_size);

#endif
