#ifndef TRANSIT_PROTOCOL_H
#define TRANSIT_PROTOCOL_H

// 서버 버전 표시용 문자열입니다.
#define SERVER_VERSION "1.0.0"

// 통신 문자열의 최대 크기와 필드 크기를 정합니다.
#define MAX_PACKET_SIZE 32768
#define MAX_FIELD_COUNT 16
#define MAX_FIELD_LEN 512

// 요청과 응답에서 사용하는 구분자입니다.
#define FIELD_SEP '|'
#define ITEM_SEP ':'
#define LIST_SEP ';'
#define PACKET_TERM '\n'

// 접속 확인용 패킷입니다.
#define PKT_HELLO "HELLO"
#define PKT_HELLO_RES "HELLO_RES"
#define PKT_PING "PING"
#define PKT_PONG "PONG"

// 검색 요청과 응답 패킷입니다.
#define PKT_STOP_SEARCH_REQ "STOP_SEARCH_REQ"
#define PKT_STOP_SEARCH_RES "STOP_SEARCH_RES"
#define PKT_BUS_SEARCH_REQ "BUS_SEARCH_REQ"
#define PKT_BUS_SEARCH_RES "BUS_SEARCH_RES"
#define PKT_SUBWAY_SEARCH_REQ "SUBWAY_SEARCH_REQ"
#define PKT_SUBWAY_SEARCH_RES "SUBWAY_SEARCH_RES"

// 길찾기와 편의시설 요청 패킷입니다.
#define PKT_TRANSFER_REQ "TRANSFER_REQ"
#define PKT_TRANSFER_RES "TRANSFER_RES"
#define PKT_FACILITY_REQ "FACILITY_REQ"
#define PKT_FACILITY_RES "FACILITY_RES"

// 운행 정보 요청 패킷입니다.
#define PKT_ARRIVAL_REQ "ARRIVAL_REQ"
#define PKT_ARRIVAL_RES "ARRIVAL_RES"
#define PKT_CROWDING_REQ "CROWDING_REQ"
#define PKT_CROWDING_RES "CROWDING_RES"

// 즐겨찾기와 최근 검색 기록 패킷입니다.
#define PKT_FAVORITE_ADD_REQ "FAVORITE_ADD_REQ"
#define PKT_FAVORITE_ADD_RES "FAVORITE_ADD_RES"
#define PKT_FAVORITE_LIST_REQ "FAVORITE_LIST_REQ"
#define PKT_FAVORITE_LIST_RES "FAVORITE_LIST_RES"
#define PKT_FAVORITE_DELETE_REQ "FAVORITE_DELETE_REQ"
#define PKT_FAVORITE_DELETE_RES "FAVORITE_DELETE_RES"
#define PKT_RECENT_LIST_REQ "RECENT_LIST_REQ"
#define PKT_RECENT_LIST_RES "RECENT_LIST_RES"

#define PKT_ERROR "ERROR"

// 서버 처리 결과 코드입니다.
typedef enum ErrorCode {
    ERR_OK = 0,              // 정상 처리
    ERR_NOT_FOUND = 1,       // 검색 결과 없음
    ERR_INVALID_REQUEST = 2, // 잘못된 요청
    ERR_DATA_LOAD_ERROR = 3, // 데이터 로드 실패
    ERR_SERVER_ERROR = 4     // 서버 내부 오류
} ErrorCode;

#endif
