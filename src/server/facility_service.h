#ifndef TRANSIT_FACILITY_SERVICE_H
#define TRANSIT_FACILITY_SERVICE_H

#include "types.h"

// 위치와 시설 종류 조건을 기준으로 주변 편의시설 응답을 생성합니다.
int handle_facility(AppData *data, const char *payload, char *response, unsigned long response_size);

#endif
