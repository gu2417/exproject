#ifndef TRANSIT_SEARCH_SERVICE_H
#define TRANSIT_SEARCH_SERVICE_H

#include "types.h"

// 정류장·버스·지하철역 검색 요청에 대한 응답 생성 함수를 제공합니다.
int handle_stop_search(AppData *data, const char *keyword, char *response, unsigned long response_size);
int handle_bus_search(AppData *data, const char *route_no, char *response, unsigned long response_size);
int handle_subway_search(AppData *data, const char *keyword, char *response, unsigned long response_size);

#endif
