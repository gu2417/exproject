#ifndef TRANSIT_TRANSFER_SERVICE_H
#define TRANSIT_TRANSFER_SERVICE_H

#include "types.h"

// 출발지·도착지 기반 최단 경로 요청을 처리합니다.
int handle_transfer(AppData *data, const char *payload, char *response, unsigned long response_size);

#endif
