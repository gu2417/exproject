#ifndef TRANSIT_ROUTER_H
#define TRANSIT_ROUTER_H

#include "types.h"

// 요청 패킷 종류에 따라 적절한 서비스 함수로 분기합니다.
int route_request(AppData *data, const char *request, char *response, unsigned long response_size);

#endif
