#include "data_loader.h"

#include "csv_parser.h"
#include "graph.h"
#include "linked_list.h"
#include "log.h"
#include "string_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// CSV 숫자 문자열을 실수형으로 변환하고 빈 값은 기본값 0으로 보정합니다.
static double parse_double_or_zero(const char *s) {
    // CSV의 빈 숫자 칸은 기본값 0으로 보정합니다.
    if (!s || !*s) {
        return 0.0;
    }
    return atof(s);
}

// CSV 숫자 문자열을 정수형으로 변환하고 빈 값은 기본값 0으로 보정합니다.
static int parse_int_or_zero(const char *s) {
    // CSV의 빈 숫자 칸은 기본값 0으로 보정합니다.
    if (!s || !*s) {
        return 0;
    }
    return atoi(s);
}

// CSV 파일을 엽니다.
static int open_csv(const char *path, FILE **out) {
    char fallback_path[512];

    // 현재 작업 폴더 기준으로 먼저 열어 봅니다.
    *out = fopen(path, "r");
    if (!*out) {
        // 실행 위치가 bin 폴더일 수도 있어서 상위 폴더 기준도 확인합니다.
        snprintf(fallback_path, sizeof(fallback_path), "../%s", path);
        *out = fopen(fallback_path, "r");
    }
    // 두 위치 모두 실패하면 로그를 남깁니다.
    if (!*out) {
        log_error("-", "DATA_LOAD", 3, path);
        return 0;
    }
    return 1;
}

// 정류장 이름으로 쓰기 어려운 값인지 확인합니다.
static int is_placeholder_stop_name(const char *name) {
    return str_is_blank(name) || strcmp(name, "0") == 0 || strcmp(name, "1") == 0;
}

// 버스 정류장 CSV를 읽어 정류장 목록을 구성합니다.
static int load_bus_stops(AppData *data) {
    FILE *fp;
    char line[1024];
    CsvRow header;
    int id_i, name_i, lat_i, lng_i, district_i;
    int loaded = 0;

    // 정류장 파일을 엽니다.
    if (!open_csv("data/bus_stops.csv", &fp)) {
        return 0;
    }
    // 첫 번째 행은 데이터 컬럼명을 담은 헤더로 사용합니다.
    if (!fgets(line, sizeof(line), fp) || !csv_parse_line(line, &header)) {
        fclose(fp);
        return 0;
    }
    // 헤더에서 필요한 컬럼 인덱스를 확보합니다.
    id_i = csv_find_header(&header, "stop_id");
    name_i = csv_find_header(&header, "stop_name");
    lat_i = csv_find_header(&header, "lat");
    lng_i = csv_find_header(&header, "lng");
    district_i = csv_find_header(&header, "district");
    // 정류장 ID와 이름은 필수입니다.
    if (id_i < 0 || name_i < 0) {
        fclose(fp);
        log_error("-", "DATA_LOAD", 3, "bus_stops.csv missing required headers");
        return 0;
    }

    // 헤더 이후의 행을 순회하며 정류장 데이터를 적재합니다.
    while (fgets(line, sizeof(line), fp)) {
        CsvRow row;
        StopNode stop;
        // CSV 행이 정상적으로 해석되고 필수 ID가 확인된 경우에만 정류장으로 등록합니다.
        if (!csv_parse_line(line, &row) || str_is_blank(csv_get(&row, id_i))) {
            continue;
        }
        // 정류장 구조체를 비우고 CSV 값을 채웁니다.
        memset(&stop, 0, sizeof(stop));
        safe_strcpy(stop.stop_id, sizeof(stop.stop_id), csv_get(&row, id_i));
        safe_strcpy(stop.stop_name, sizeof(stop.stop_name), csv_get(&row, name_i));
        // 이름이 없거나 숫자뿐이면 임시 이름을 넣습니다.
        if (is_placeholder_stop_name(stop.stop_name)) {
            snprintf(stop.stop_name, sizeof(stop.stop_name), "정류장명 미상(%s)", stop.stop_id);
        }
        stop.lat = parse_double_or_zero(csv_get(&row, lat_i));
        stop.lng = parse_double_or_zero(csv_get(&row, lng_i));
        safe_strcpy(stop.district, sizeof(stop.district), csv_get(&row, district_i));
        // 같은 ID가 없을 때만 목록에 추가합니다.
        if (!find_stop_by_id(data, stop.stop_id)) {
            stop_add(data, &stop);
            loaded++;
        }
    }
    // 파일을 닫고 로딩 로그를 남깁니다.
    fclose(fp);
    log_info("-", "DATA_LOAD", 0, "bus_stops.csv loaded");
    return loaded > 0;
}

// 버스 노선 CSV를 읽어 노선 기본 정보를 적재합니다.
static int load_bus_routes(AppData *data) {
    FILE *fp;
    char line[1024];
    CsvRow header;
    int no_i, name_i, start_i, end_i, interval_i;
    int loaded = 0;

    // 노선 파일을 엽니다.
    if (!open_csv("data/bus_routes.csv", &fp)) {
        return 0;
    }
    // 첫 번째 행은 데이터 컬럼명을 담은 헤더로 사용합니다.
    if (!fgets(line, sizeof(line), fp) || !csv_parse_line(line, &header)) {
        fclose(fp);
        return 0;
    }
    // 헤더에서 필요한 컬럼 인덱스를 확보합니다.
    no_i = csv_find_header(&header, "route_no");
    name_i = csv_find_header(&header, "route_name");
    start_i = csv_find_header(&header, "start_stop_id");
    end_i = csv_find_header(&header, "end_stop_id");
    interval_i = csv_find_header(&header, "interval_min");
    // 노선 번호, 출발 정류장, 종점 정류장은 필수입니다.
    if (no_i < 0 || start_i < 0 || end_i < 0) {
        fclose(fp);
        log_error("-", "DATA_LOAD", 3, "bus_routes.csv missing required headers");
        return 0;
    }

    // 각 행을 순회하며 노선 구조체에 필요한 값을 채웁니다.
    while (fgets(line, sizeof(line), fp)) {
        CsvRow row;
        RouteNode route;
        // 노선 번호가 확인되는 정상 행만 노선 목록에 반영합니다.
        if (!csv_parse_line(line, &row) || str_is_blank(csv_get(&row, no_i))) {
            continue;
        }
        // 노선 구조체를 비우고 값을 채웁니다.
        memset(&route, 0, sizeof(route));
        safe_strcpy(route.route_no, sizeof(route.route_no), csv_get(&row, no_i));
        safe_strcpy(route.route_name, sizeof(route.route_name), csv_get(&row, name_i));
        safe_strcpy(route.start_stop_id, sizeof(route.start_stop_id), csv_get(&row, start_i));
        safe_strcpy(route.end_stop_id, sizeof(route.end_stop_id), csv_get(&row, end_i));
        route.interval_min = parse_int_or_zero(csv_get(&row, interval_i));
        // 같은 노선 번호가 없을 때만 추가합니다.
        if (!find_route_by_no(data, route.route_no)) {
            route_add(data, &route);
            loaded++;
        }
    }
    fclose(fp);
    log_info("-", "DATA_LOAD", 0, "bus_routes.csv loaded");
    return loaded > 0;
}

// 노선-정류장 CSV를 읽어 노선별 정차 순서를 연결합니다.
static int load_route_stops(AppData *data) {
    FILE *fp;
    char line[1024];
    CsvRow header;
    int route_i, seq_i, stop_i, direction_i;
    int loaded = 0;

    // 노선-정류장 파일을 엽니다.
    if (!open_csv("data/route_stops.csv", &fp)) {
        return 0;
    }
    // 첫 번째 행은 데이터 컬럼명을 담은 헤더로 사용합니다.
    if (!fgets(line, sizeof(line), fp) || !csv_parse_line(line, &header)) {
        fclose(fp);
        return 0;
    }
    // 헤더에서 필요한 컬럼 인덱스를 확보합니다.
    route_i = csv_find_header(&header, "route_no");
    seq_i = csv_find_header(&header, "seq");
    stop_i = csv_find_header(&header, "stop_id");
    direction_i = csv_find_header(&header, "direction");
    // 노선 번호, 순서, 정류장 ID는 필수입니다.
    if (route_i < 0 || seq_i < 0 || stop_i < 0) {
        fclose(fp);
        log_error("-", "DATA_LOAD", 3, "route_stops.csv missing required headers");
        return 0;
    }

    // 각 행을 순회하며 노선 안의 정류장 순서를 등록합니다.
    while (fgets(line, sizeof(line), fp)) {
        CsvRow row;
        RouteStopNode rs;
        RouteNode *route;
        // CSV 행을 해석하지 못한 경우 해당 행은 로딩 대상에서 제외합니다.
        if (!csv_parse_line(line, &row)) {
            continue;
        }
        // 노선 번호와 정류장 ID가 모두 있는 행만 노선별 정류장으로 등록합니다.
        if (str_is_blank(csv_get(&row, route_i)) || str_is_blank(csv_get(&row, stop_i))) {
            continue;
        }
        // 실제로 읽어 둔 노선과 정류장인지 확인합니다.
        route = find_route_by_no(data, csv_get(&row, route_i));
        if (!route || !find_stop_by_id(data, csv_get(&row, stop_i))) {
            log_error("-", "DATA_LOAD", 3, "route_stops.csv has invalid route or stop reference");
            continue;
        }
        // 노선 정류장 구조체를 채웁니다.
        memset(&rs, 0, sizeof(rs));
        safe_strcpy(rs.route_no, sizeof(rs.route_no), csv_get(&row, route_i));
        rs.seq = parse_int_or_zero(csv_get(&row, seq_i));
        safe_strcpy(rs.stop_id, sizeof(rs.stop_id), csv_get(&row, stop_i));
        safe_strcpy(rs.direction, sizeof(rs.direction), csv_get(&row, direction_i));
        // 해당 노선의 정류장 목록에 추가합니다.
        route_stop_add(route, &rs);
        loaded++;
    }
    fclose(fp);
    log_info("-", "DATA_LOAD", 0, "route_stops.csv loaded");
    return loaded > 0;
}

// 지하철역 CSV를 읽어 역 목록을 구성합니다.
static int load_subway_stations(AppData *data) {
    FILE *fp;
    char line[1024];
    CsvRow header;
    int id_i, name_i, line_i, lat_i, lng_i, address_i, transfer_i;
    int loaded = 0;

    // 지하철역 파일을 엽니다.
    if (!open_csv("data/subway_stations.csv", &fp)) {
        return 0;
    }
    // 첫 번째 행은 데이터 컬럼명을 담은 헤더로 사용합니다.
    if (!fgets(line, sizeof(line), fp) || !csv_parse_line(line, &header)) {
        fclose(fp);
        return 0;
    }
    // 헤더에서 필요한 컬럼 인덱스를 확보합니다.
    id_i = csv_find_header(&header, "station_id");
    name_i = csv_find_header(&header, "station_name");
    line_i = csv_find_header(&header, "line_no");
    lat_i = csv_find_header(&header, "lat");
    lng_i = csv_find_header(&header, "lng");
    address_i = csv_find_header(&header, "address");
    transfer_i = csv_find_header(&header, "transfer_lines");
    // 역 ID, 역명, 노선명은 필수입니다.
    if (id_i < 0 || name_i < 0 || line_i < 0) {
        fclose(fp);
        log_error("-", "DATA_LOAD", 3, "subway_stations.csv missing required headers");
        return 0;
    }

    // 각 행을 순회하며 역 ID, 이름, 노선 정보를 적재합니다.
    while (fgets(line, sizeof(line), fp)) {
        CsvRow row;
        StationNode station;
        // 역 ID가 확인되는 정상 행만 지하철역 데이터로 등록합니다.
        if (!csv_parse_line(line, &row) || str_is_blank(csv_get(&row, id_i))) {
            continue;
        }
        // 역 구조체를 비우고 값을 채웁니다.
        memset(&station, 0, sizeof(station));
        safe_strcpy(station.station_id, sizeof(station.station_id), csv_get(&row, id_i));
        safe_strcpy(station.station_name, sizeof(station.station_name), csv_get(&row, name_i));
        safe_strcpy(station.line_no, sizeof(station.line_no), csv_get(&row, line_i));
        station.lat = parse_double_or_zero(csv_get(&row, lat_i));
        station.lng = parse_double_or_zero(csv_get(&row, lng_i));
        safe_strcpy(station.address, sizeof(station.address), csv_get(&row, address_i));
        safe_strcpy(station.transfer_lines, sizeof(station.transfer_lines), csv_get(&row, transfer_i));
        // 같은 역 ID가 없을 때만 추가합니다.
        if (!find_station_by_id(data, station.station_id)) {
            station_add(data, &station);
            loaded++;
        }
    }
    fclose(fp);
    log_info("-", "DATA_LOAD", 0, "subway_stations.csv loaded");
    return loaded > 0;
}

// 지하철 거리로 예상 시간을 계산합니다.
static int subway_minutes_from_distance(double distance_km) {
    int minutes;
    // 거리 정보가 누락된 지하철 구간은 기본 소요 시간 2분을 적용합니다.
    if (distance_km <= 0.0) {
        return 2;
    }
    // 1km에 약 2분 정도로 계산합니다.
    minutes = (int)(distance_km * 2.0 + 0.5);
    return minutes < 2 ? 2 : minutes;
}

// 지하철 역간 거리 CSV를 읽어 역 사이 이동 정보를 구성합니다.
static int load_subway_distances(AppData *data) {
    FILE *fp;
    char line[1024];
    CsvRow header;
    int from_i, to_i, line_i, distance_i, minutes_i;
    int loaded = 0;

    // 역간 거리 파일을 엽니다.
    if (!open_csv("data/subway_distances.csv", &fp)) {
        return 0;
    }
    // 첫 번째 행은 데이터 컬럼명을 담은 헤더로 사용합니다.
    if (!fgets(line, sizeof(line), fp) || !csv_parse_line(line, &header)) {
        fclose(fp);
        return 0;
    }
    // 헤더에서 필요한 컬럼 인덱스를 확보합니다.
    from_i = csv_find_header(&header, "from_station_id");
    to_i = csv_find_header(&header, "to_station_id");
    line_i = csv_find_header(&header, "line_no");
    distance_i = csv_find_header(&header, "distance_km");
    minutes_i = csv_find_header(&header, "estimated_minutes");
    // 출발역, 도착역, 거리는 필수입니다.
    if (from_i < 0 || to_i < 0 || distance_i < 0) {
        fclose(fp);
        log_error("-", "DATA_LOAD", 3, "subway_distances.csv missing required headers");
        return 0;
    }

    // 각 행을 순회하며 역 사이 거리와 예상 시간을 적재합니다.
    while (fgets(line, sizeof(line), fp)) {
        CsvRow row;
        SubwayDistanceNode distance;
        StationNode *from_station;
        StationNode *to_station;

        // 역 ID가 확인되는 정상 행만 지하철역 데이터로 등록합니다.
        if (!csv_parse_line(line, &row) || str_is_blank(csv_get(&row, from_i)) || str_is_blank(csv_get(&row, to_i))) {
            continue;
        }

        // 실제로 읽어 둔 역인지 확인합니다.
        from_station = find_station_by_id(data, csv_get(&row, from_i));
        to_station = find_station_by_id(data, csv_get(&row, to_i));
        if (!from_station || !to_station) {
            log_error("-", "DATA_LOAD", 3, "subway_distances.csv has invalid station reference");
            continue;
        }

        // 역간 거리 구조체를 채웁니다.
        memset(&distance, 0, sizeof(distance));
        safe_strcpy(distance.from_station_id, sizeof(distance.from_station_id), csv_get(&row, from_i));
        safe_strcpy(distance.to_station_id, sizeof(distance.to_station_id), csv_get(&row, to_i));
        // CSV에 노선명이 있으면 우선 사용하고, 누락 시 출발역의 노선명으로 보완합니다.
        if (line_i >= 0 && !str_is_blank(csv_get(&row, line_i))) {
            safe_strcpy(distance.line_no, sizeof(distance.line_no), csv_get(&row, line_i));
        } else {
            safe_strcpy(distance.line_no, sizeof(distance.line_no), from_station->line_no);
        }
        distance.distance_km = parse_double_or_zero(csv_get(&row, distance_i));
        distance.estimated_minutes = minutes_i >= 0 ? parse_int_or_zero(csv_get(&row, minutes_i)) : 0;
        // 예상 시간이 비어 있으면 역간 거리 값으로 소요 시간을 산정합니다.
        if (distance.estimated_minutes <= 0) {
            distance.estimated_minutes = subway_minutes_from_distance(distance.distance_km);
        }

        // 역간 거리 목록에 추가합니다.
        subway_distance_add(data, &distance);
        loaded++;
    }
    fclose(fp);
    log_info("-", "DATA_LOAD", 0, "subway_distances.csv loaded");
    return loaded > 0;
}

// 편의시설 CSV를 읽어 위치 기반 시설 목록을 구성합니다.
static int load_facilities(AppData *data) {
    FILE *fp;
    char line[1024];
    CsvRow header;
    int id_i, name_i, category_i, node_i, address_i, lat_i, lng_i;
    int loaded = 0;

    // 편의시설 파일을 엽니다.
    if (!open_csv("data/facilities.csv", &fp)) {
        return 0;
    }
    // 첫 번째 행은 데이터 컬럼명을 담은 헤더로 사용합니다.
    if (!fgets(line, sizeof(line), fp) || !csv_parse_line(line, &header)) {
        fclose(fp);
        return 0;
    }
    // 헤더에서 필요한 컬럼 인덱스를 확보합니다.
    id_i = csv_find_header(&header, "facility_id");
    name_i = csv_find_header(&header, "name");
    category_i = csv_find_header(&header, "category");
    node_i = csv_find_header(&header, "nearest_node_id");
    address_i = csv_find_header(&header, "address");
    lat_i = csv_find_header(&header, "lat");
    lng_i = csv_find_header(&header, "lng");
    // 시설 ID, 이름, 종류, 가까운 노드 ID는 필수입니다.
    if (id_i < 0 || name_i < 0 || category_i < 0 || node_i < 0) {
        fclose(fp);
        log_error("-", "DATA_LOAD", 3, "facilities.csv missing required headers");
        return 0;
    }

    // 각 행을 순회하며 시설명, 종류, 주소, 연결 노드를 적재합니다.
    while (fgets(line, sizeof(line), fp)) {
        CsvRow row;
        FacilityNode facility;
        // 시설 ID가 확인되는 정상 행만 편의시설 목록에 반영합니다.
        if (!csv_parse_line(line, &row) || str_is_blank(csv_get(&row, id_i))) {
            continue;
        }
        // 편의시설 구조체를 채웁니다.
        memset(&facility, 0, sizeof(facility));
        safe_strcpy(facility.facility_id, sizeof(facility.facility_id), csv_get(&row, id_i));
        safe_strcpy(facility.name, sizeof(facility.name), csv_get(&row, name_i));
        safe_strcpy(facility.category, sizeof(facility.category), csv_get(&row, category_i));
        safe_strcpy(facility.nearest_node_id, sizeof(facility.nearest_node_id), csv_get(&row, node_i));
        safe_strcpy(facility.address, sizeof(facility.address), csv_get(&row, address_i));
        facility.lat = parse_double_or_zero(csv_get(&row, lat_i));
        facility.lng = parse_double_or_zero(csv_get(&row, lng_i));
        // 편의시설 목록에 추가합니다.
        facility_add(data, &facility);
        loaded++;
    }
    fclose(fp);
    log_info("-", "DATA_LOAD", 0, "facilities.csv loaded");
    return loaded >= 0;
}

// 도착 예정 시간 CSV를 읽어 운행 정보 테스트 데이터를 구성합니다.
static int load_arrivals(AppData *data) {
    FILE *fp;
    char line[1024];
    CsvRow header;
    int route_i, node_i, slot_i, minutes_i, status_i;

    // 도착 예정 시간 파일을 엽니다.
    if (!open_csv("data/arrival_test.csv", &fp)) {
        return 0;
    }
    // 첫 번째 행은 데이터 컬럼명을 담은 헤더로 사용합니다.
    if (!fgets(line, sizeof(line), fp) || !csv_parse_line(line, &header)) {
        fclose(fp);
        return 0;
    }
    // 헤더에서 필요한 컬럼 인덱스를 확보합니다.
    route_i = csv_find_header(&header, "route_no");
    node_i = csv_find_header(&header, "node_id");
    slot_i = csv_find_header(&header, "time_slot");
    minutes_i = csv_find_header(&header, "arrival_minutes");
    status_i = csv_find_header(&header, "status");
    // 필수 컬럼을 찾지 못하면 해당 CSV 로딩을 실패로 반환합니다.
    if (route_i < 0 || node_i < 0 || slot_i < 0 || minutes_i < 0 || status_i < 0) {
        fclose(fp);
        return 0;
    }
    // 각 행을 순회하며 노드, 노선, 시간대별 도착 정보를 적재합니다.
    while (fgets(line, sizeof(line), fp)) {
        CsvRow row;
        ArrivalNode arrival;
        // 행 구조가 맞지 않으면 테스트 데이터 목록에 반영하지 않습니다.
        if (!csv_parse_line(line, &row)) {
            continue;
        }
        // 도착 정보 구조체를 채웁니다.
        memset(&arrival, 0, sizeof(arrival));
        safe_strcpy(arrival.route_no, sizeof(arrival.route_no), csv_get(&row, route_i));
        safe_strcpy(arrival.node_id, sizeof(arrival.node_id), csv_get(&row, node_i));
        safe_strcpy(arrival.time_slot, sizeof(arrival.time_slot), csv_get(&row, slot_i));
        arrival.arrival_minutes = parse_int_or_zero(csv_get(&row, minutes_i));
        safe_strcpy(arrival.status, sizeof(arrival.status), csv_get(&row, status_i));
        // 도착 정보 목록에 추가합니다.
        arrival_add(data, &arrival);
    }
    fclose(fp);
    log_info("-", "DATA_LOAD", 0, "arrival_test.csv loaded");
    return 1;
}

// 혼잡도 CSV를 읽어 시간대별 혼잡도 데이터를 구성합니다.
static int load_crowdings(AppData *data) {
    FILE *fp;
    char line[1024];
    CsvRow header;
    int route_i, node_i, slot_i, level_i;

    // 혼잡도 파일을 엽니다.
    if (!open_csv("data/crowding_test.csv", &fp)) {
        return 0;
    }
    // 첫 번째 행은 데이터 컬럼명을 담은 헤더로 사용합니다.
    if (!fgets(line, sizeof(line), fp) || !csv_parse_line(line, &header)) {
        fclose(fp);
        return 0;
    }
    // 헤더에서 필요한 컬럼 인덱스를 확보합니다.
    route_i = csv_find_header(&header, "route_no");
    node_i = csv_find_header(&header, "node_id");
    slot_i = csv_find_header(&header, "time_slot");
    level_i = csv_find_header(&header, "crowding_level");
    // 필수 컬럼을 찾지 못하면 해당 CSV 로딩을 실패로 반환합니다.
    if (route_i < 0 || node_i < 0 || slot_i < 0 || level_i < 0) {
        fclose(fp);
        return 0;
    }
    // 각 행을 순회하며 노드와 노선별 혼잡도 정보를 적재합니다.
    while (fgets(line, sizeof(line), fp)) {
        CsvRow row;
        CrowdingNode crowding;
        // 행 구조가 맞지 않으면 테스트 데이터 목록에 반영하지 않습니다.
        if (!csv_parse_line(line, &row)) {
            continue;
        }
        // 혼잡도 구조체를 채웁니다.
        memset(&crowding, 0, sizeof(crowding));
        safe_strcpy(crowding.route_no, sizeof(crowding.route_no), csv_get(&row, route_i));
        safe_strcpy(crowding.node_id, sizeof(crowding.node_id), csv_get(&row, node_i));
        safe_strcpy(crowding.time_slot, sizeof(crowding.time_slot), csv_get(&row, slot_i));
        safe_strcpy(crowding.crowding_level, sizeof(crowding.crowding_level), csv_get(&row, level_i));
        // 혼잡도 목록에 추가합니다.
        crowding_add(data, &crowding);
    }
    fclose(fp);
    log_info("-", "DATA_LOAD", 0, "crowding_test.csv loaded");
    return 1;
}

static int has_coordinates(double lat, double lng) {
    // 좌표가 모두 0인 데이터는 위치값이 누락된 항목으로 판단합니다.
    return lat != 0.0 && lng != 0.0;
}

// 위도와 경도 차이를 대략적인 거리 제곱값으로 바꿉니다.
static double distance_sq_km(double lat1, double lng1, double lat2, double lng2) {
    // 위도 1도는 약 111km로 계산합니다.
    double lat_km = (lat1 - lat2) * 111.0;
    // 경도는 이 지역 기준으로 약 88km로 계산합니다.
    double lng_km = (lng1 - lng2) * 88.0;
    return lat_km * lat_km + lng_km * lng_km;
}

// 거리 제곱값으로 환승/도보 예상 시간을 정합니다.
static int transfer_minutes_from_distance(double distance_sq) {
    // 거리 구간에 따라 도보 환승 시간을 단계적으로 산정합니다.
    if (distance_sq <= 0.09) {
        return 3;
    }
    if (distance_sq <= 0.25) {
        return 5;
    }
    if (distance_sq <= 1.00) {
        return 10;
    }
    if (distance_sq <= 2.25) {
        return 15;
    }
    if (distance_sq <= 4.00) {
        return 20;
    }
    if (distance_sq <= 9.00) {
        return 30;
    }
    if (distance_sq <= 25.00) {
        return 50;
    }
    return 80;
}

// 쉼표로 구분된 문자열에서 다음 값을 하나 꺼냅니다.
static int next_csv_token(const char **cursor, char *out, size_t out_size) {
    const char *start;
    const char *end;
    size_t len;

    // 토큰을 읽을 위치와 결과 버퍼가 준비되어야 값을 분리합니다.
    if (!cursor || !*cursor || !out || out_size == 0) {
        return 0;
    }

    // 토큰 시작 위치가 나올 때까지 선행 공백을 이동합니다.
    start = *cursor;
    while (*start == ' ') {
        start++;
    }
    // 현재 위치에 토큰이 남아 있지 않으면 분리를 중단합니다.
    if (*start == '\0') {
        return 0;
    }

    // 쉼표나 문자열 끝까지 이동합니다.
    end = start;
    while (*end && *end != ',') {
        end++;
    }

    // 저장 공간보다 길면 잘라서 복사합니다.
    len = (size_t)(end - start);
    if (len >= out_size) {
        len = out_size - 1;
    }
    // 토큰 값을 복사하고 앞뒤 빈칸을 정리합니다.
    memcpy(out, start, len);
    out[len] = '\0';
    str_trim(out);

    // 다음 호출이 이어서 읽을 수 있게 위치를 옮깁니다.
    *cursor = (*end == ',') ? end + 1 : end;
    return !str_is_blank(out);
}

// 두 노선 문자열에서 공통으로 들어 있는 노선명을 찾습니다.
static int common_line_name(const char *a, const char *b, char *out, size_t out_size) {
    const char *a_cursor = a;
    char a_token[MAX_ROUTE_LEN];

    // 두 노선 문자열과 결과 버퍼가 모두 있어야 공통 노선을 비교합니다.
    if (!a || !b || !out || out_size == 0) {
        return 0;
    }

    // 첫 번째 문자열의 노선명을 하나씩 꺼냅니다.
    while (next_csv_token(&a_cursor, a_token, sizeof(a_token))) {
        const char *b_cursor = b;
        char b_token[MAX_ROUTE_LEN];
        // 두 번째 문자열의 노선명과 비교합니다.
        while (next_csv_token(&b_cursor, b_token, sizeof(b_token))) {
            if (strcmp(a_token, b_token) == 0) {
                safe_strcpy(out, out_size, a_token);
                return 1;
            }
        }
    }
    return 0;
}

// 정류장과 역 정보를 그래프에 넣습니다.
int data_loader_build_graph(AppData *data) {
    StopNode *stop;
    StationNode *station;
    RouteNode *route;
    int loaded_subway_edges = 0;

    // 로드된 정류장과 역 데이터가 있어야 환승 그래프를 구성할 수 있습니다.
    if (!data) {
        return 0;
    }

    // 모든 버스 정류장을 그래프 노드로 넣습니다.
    for (stop = data->stops; stop; stop = stop->next) {
        graph_add_node(&data->graph, stop->stop_id, stop->stop_name, "bus_stop", stop->lat, stop->lng);
    }
    // 모든 지하철역을 그래프 노드로 넣습니다.
    for (station = data->stations; station; station = station->next) {
        graph_add_node(&data->graph, station->station_id, station->station_name, "subway_station", station->lat, station->lng);
    }

    // 노선 정차 순서를 따라 인접 정류장 사이의 버스 간선을 생성합니다.
    for (route = data->routes; route; route = route->next) {
        RouteStopNode *prev = NULL;
        RouteStopNode *cur;
        for (cur = route->stops; cur; cur = cur->next) {
            // 이전 정류장과 현재 정류장을 양방향으로 연결합니다.
            if (prev) {
                int a = graph_find_node(&data->graph, prev->stop_id);
                int b = graph_find_node(&data->graph, cur->stop_id);
                graph_add_edge(&data->graph, a, b, 5, route->route_no);
                graph_add_edge(&data->graph, b, a, 5, route->route_no);
            }
            prev = cur;
        }
    }

    // 역간 거리 데이터가 제공되면 실제 거리 기반으로 지하철 간선을 생성합니다.
    if (data->subway_distances) {
        SubwayDistanceNode *distance;
        for (distance = data->subway_distances; distance; distance = distance->next) {
            int a = graph_find_node(&data->graph, distance->from_station_id);
            int b = graph_find_node(&data->graph, distance->to_station_id);
            int minutes = distance->estimated_minutes > 0 ? distance->estimated_minutes : subway_minutes_from_distance(distance->distance_km);
            const char *line_no = str_is_blank(distance->line_no) ? "SUBWAY" : distance->line_no;

            // 양 끝 노드가 모두 그래프에 등록된 경우에만 간선을 생성합니다.
            if (a < 0 || b < 0) {
                continue;
            }
            // 지하철도 양방향 이동이 가능하게 연결합니다.
            graph_add_edge(&data->graph, a, b, minutes, line_no);
            graph_add_edge(&data->graph, b, a, minutes, line_no);
            loaded_subway_edges++;
        }
    }

    // 역간 거리 파일이 비어 있으면 같은 노선끼리 기본 시간으로 연결합니다.
    if (loaded_subway_edges == 0) {
        for (station = data->stations; station; station = station->next) {
            StationNode *other;
            for (other = station->next; other; other = other->next) {
                char common_line[MAX_ROUTE_LEN];
                // 두 역이 같은 노선을 가지고 있으면 연결합니다.
                if (common_line_name(station->line_no, other->line_no, common_line, sizeof(common_line))) {
                    int a = graph_find_node(&data->graph, station->station_id);
                    int b = graph_find_node(&data->graph, other->station_id);
                    graph_add_edge(&data->graph, a, b, 8, common_line);
                    graph_add_edge(&data->graph, b, a, 8, common_line);
                }
            }
        }
    }

    // 정류장과 가까운 지하철역을 환승 구간으로 연결합니다.
    for (stop = data->stops; stop; stop = stop->next) {
        StationNode *nearest_station = NULL;
        double nearest_distance = 999999.0;
        int linked_near_station = 0;

        // 좌표가 누락된 정류장은 지하철역과의 거리 비교에서 제외합니다.
        if (!has_coordinates(stop->lat, stop->lng)) {
            continue;
        }

        for (station = data->stations; station; station = station->next) {
            double distance;
            // 역 좌표가 없는 항목은 거리 비교 후보에서 제외합니다.
            if (!has_coordinates(station->lat, station->lng)) {
                continue;
            }

            // 정류장과 역 사이의 대략적인 거리 제곱값을 계산합니다.
            distance = distance_sq_km(stop->lat, stop->lng, station->lat, station->lng);
            // 가장 가까운 역을 계속 갱신합니다.
            if (distance < nearest_distance) {
                nearest_distance = distance;
                nearest_station = station;
            }

            // 일정 거리 안에 있는 역은 바로 환승 간선으로 연결합니다.
            if (distance <= 9.0) {
                int a = graph_find_node(&data->graph, stop->stop_id);
                int b = graph_find_node(&data->graph, station->station_id);
                int minutes = transfer_minutes_from_distance(distance);
                // 정류장과 역을 양방향 환승 구간으로 연결합니다.
                graph_add_edge(&data->graph, a, b, minutes, "TRANSFER");
                graph_add_edge(&data->graph, b, a, minutes, "TRANSFER");
                linked_near_station = 1;
            }
        }

        // 가까운 역이 하나도 연결되지 않았으면 가장 가까운 역 하나라도 연결합니다.
        if (!linked_near_station && nearest_station) {
            int a = graph_find_node(&data->graph, stop->stop_id);
            int b = graph_find_node(&data->graph, nearest_station->station_id);
            int minutes = transfer_minutes_from_distance(nearest_distance);
            graph_add_edge(&data->graph, a, b, minutes, "TRANSFER");
            graph_add_edge(&data->graph, b, a, minutes, "TRANSFER");
        }
    }

    // 그래프 생성 완료를 로그에 남깁니다.
    log_info("-", "DATA_LOAD", 0, "transfer graph built");
    return data->graph.node_count > 0;
}

// 여러 CSV 원본을 읽어 프로그램에서 사용할 통합 데이터 구조를 구성합니다.
int data_loader_load(AppData *data) {
    int ok = 1;
    // 데이터 저장 구조체가 유효할 때만 CSV 로딩 절차를 시작합니다.
    if (!data) {
        return 0;
    }
    // 각 CSV를 차례대로 읽고, 하나라도 실패하면 ok 값이 0으로 남습니다.
    ok = load_bus_stops(data) && ok;
    ok = load_bus_routes(data) && ok;
    ok = load_route_stops(data) && ok;
    ok = load_subway_stations(data) && ok;
    ok = load_subway_distances(data) && ok;
    ok = load_facilities(data) && ok;
    ok = load_arrivals(data) && ok;
    ok = load_crowdings(data) && ok;
    // 적재된 정류장·역 데이터를 바탕으로 길찾기용 그래프를 구성합니다.
    ok = data_loader_build_graph(data) && ok;
    return ok;
}
