#ifndef TRANSIT_LINKED_LIST_H
#define TRANSIT_LINKED_LIST_H

#include "types.h"

// 전체 데이터 저장 공간을 준비하고 해제합니다.
void app_data_init(AppData *data);
void app_data_free(AppData *data);

// CSV에서 읽은 데이터를 연결 리스트에 추가합니다.
StopNode *stop_add(AppData *data, const StopNode *value);
RouteNode *route_add(AppData *data, const RouteNode *value);
RouteStopNode *route_stop_add(RouteNode *route, const RouteStopNode *value);
StationNode *station_add(AppData *data, const StationNode *value);
SubwayDistanceNode *subway_distance_add(AppData *data, const SubwayDistanceNode *value);
FacilityNode *facility_add(AppData *data, const FacilityNode *value);
ArrivalNode *arrival_add(AppData *data, const ArrivalNode *value);
CrowdingNode *crowding_add(AppData *data, const CrowdingNode *value);

// ID나 조건으로 저장된 데이터를 찾습니다.
StopNode *find_stop_by_id(const AppData *data, const char *stop_id);
RouteNode *find_route_by_no(const AppData *data, const char *route_no);
StationNode *find_station_by_id(const AppData *data, const char *station_id);
const char *find_node_name_by_id(const AppData *data, const char *node_id);
ArrivalNode *find_arrival(const AppData *data, const char *node_id, const char *route_no, const char *time_slot);
CrowdingNode *find_crowding(const AppData *data, const char *node_id, const char *route_no, const char *time_slot);
int route_has_stop(const RouteNode *route, const char *stop_id);

#endif
