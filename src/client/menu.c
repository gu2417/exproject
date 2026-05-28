#include "menu.h"

#include "client_net.h"
#include "favorite.h"
#include "protocol.h"
#include "recent_search.h"
#include "string_utils.h"

#include <stdio.h>
#include <string.h>

static int read_line_prompt(const char *prompt, char *buf, unsigned long size) {
    // 입력 전에 사용자에게 보여줄 입력 안내문를 출력합니다.
    printf("%s", prompt);
    // 콘솔 입력을 읽지 못하면 메뉴 처리를 이어가지 않습니다.
    if (!fgets(buf, (int)size, stdin)) {
        return 0;
    }
    // 입력값 비교가 안정적으로 되도록 개행과 양쪽 공백을 정리합니다.
    strip_newline(buf);
    str_trim(buf);
    return 1;
}

static void print_menu(void) {
    // 메인 기능 번호를 콘솔 메뉴 형태로 출력합니다.
    printf("\n===== 아산/천안 대중교통 정보 시스템 =====\n\n");
    printf("1. 정류장 검색\n");
    printf("2. 버스 번호 검색\n");
    printf("3. 지하철역 검색\n");
    printf("4. 길찾기(최단 경로)\n");
    printf("5. 주변 편의시설 조회\n");
    printf("6. 즐겨찾기 조회\n");
    printf("7. 즐겨찾기 추가\n");
    printf("8. 즐겨찾기 삭제\n");
    printf("9. 최근 검색 기록\n");
    printf("0. 종료\n");
}

// 필수 입력값이 비어 있는 경우 메뉴 흐름을 진행하지 않습니다.
static int read_nonempty_input(const char *prompt, char *buf, unsigned long size) {
    // 입력 안내문 입력을 받은 뒤 실제 내용이 있는지 검사합니다.
    if (!read_line_prompt(prompt, buf, size) || str_is_blank(buf)) {
        printf("입력값을 다시 확인해주세요.\n");
        return 0;
    }
    return 1;
}

// 단일 검색어를 사용하는 정류장, 버스, 지하철 조회 흐름입니다.
static void run_single_keyword_menu(socket_t fd,
                                    const char *request_type,
                                    const char *prompt,
                                    char *keyword,
                                    unsigned long keyword_size) {
    char request[MAX_PACKET_SIZE];

    // 서버 조회에 사용할 검색어를 콘솔에서 입력받습니다.
    if (!read_nonempty_input(prompt, keyword, keyword_size)) {
        return;
    }

    // 메뉴 입력값을 서버 프로토콜에 맞는 요청 문자열로 변환합니다.
    snprintf(request, sizeof(request), "%s|%s\n", request_type, keyword);
    // 검색 요청을 전송하고 서버 응답을 화면 출력 함수로 넘깁니다.
    client_send_request(fd, request);
}

// 출발지와 도착지를 입력받아 최단 경로 조회 요청을 구성합니다.
static void run_route_search_menu(socket_t fd,
                                  const char *start_prompt,
                                  const char *end_prompt,
                                  char *start_name,
                                  char *end_name,
                                  unsigned long field_size) {
    char request[MAX_PACKET_SIZE];

    // 경로 탐색에 필요한 출발 위치와 도착 위치를 순서대로 받습니다.
    if (!read_line_prompt(start_prompt, start_name, field_size) ||
        !read_line_prompt(end_prompt, end_name, field_size) ||
        str_is_blank(start_name) ||
        str_is_blank(end_name)) {
        printf("입력값을 다시 확인해주세요.\n");
        return;
    }

    // 출발지와 도착지를 서버가 읽는 길찾기 패킷 형식으로 조립합니다.
    snprintf(request, sizeof(request), "%s|%s:%s\n", PKT_TRANSFER_REQ, start_name, end_name);
    // 완성된 요청 문자열을 서버로 전송합니다.
    client_send_request(fd, request);
}

// 정류장 또는 역 이름을 기준으로 주변 편의시설 조회 요청을 구성합니다.
static void run_facility_menu(socket_t fd, char *place_name, char *category, unsigned long field_size) {
    char request[MAX_PACKET_SIZE];

    // 시설 검색 기준이 되는 위치명과 카테고리를 사용자에게 입력받습니다.
    if (!read_line_prompt("정류장/역명 입력: ", place_name, field_size) ||
        !read_line_prompt("카테고리 입력: ", category, field_size) ||
        str_is_blank(place_name)) {
        printf("입력값을 다시 확인해주세요.\n");
        return;
    }
    // 시설 종류를 입력하지 않으면 전체 카테고리 검색으로 해석합니다.
    if (str_is_blank(category)) {
        safe_strcpy(category, field_size, "all");
    }

    // 위치와 카테고리를 편의시설 조회 요청 형식으로 조립합니다.
    snprintf(request, sizeof(request), "%s|%s:%s\n", PKT_FACILITY_REQ, place_name, category);
    client_send_request(fd, request);
}

// 사용자 입력을 받아 즐겨찾기 추가 요청을 생성합니다.
static void run_favorite_add_menu(socket_t fd) {
    char type[64];
    char target_value[128];
    char display_name[128];
    const char *target_prompt = "항목명 입력: ";

    // 정류장, 버스, 지하철역 중 어떤 대상을 저장할지 먼저 입력받습니다.
    printf("유형 선택: 1. 정류장  2. 버스  3. 지하철역\n");
    if (!read_line_prompt("유형 입력(번호 또는 한글): ", type, sizeof(type))) {
        return;
    }

    // 선택한 즐겨찾기 유형에 따라 대상 입력 입력 안내문를 조정합니다.
    if (strcmp(type, "1") == 0 || strcmp(type, "정류장") == 0 || strcmp(type, "버스정류장") == 0 ||
        strcmp(type, "stop") == 0) {
        target_prompt = "정류장명 입력: ";
    } else if (strcmp(type, "2") == 0 || strcmp(type, "버스") == 0 || strcmp(type, "버스번호") == 0 ||
               strcmp(type, "노선") == 0 || strcmp(type, "bus") == 0) {
        target_prompt = "버스명 입력: ";
    } else if (strcmp(type, "3") == 0 || strcmp(type, "지하철역") == 0 || strcmp(type, "역") == 0 ||
               strcmp(type, "station") == 0) {
        target_prompt = "지하철역명 입력: ";
    }

    // 서버에 저장할 대상 값과 사용자에게 보일 표시 이름을 입력받습니다.
    if (!read_line_prompt(target_prompt, target_value, sizeof(target_value)) ||
        !read_line_prompt("표시 이름 입력: ", display_name, sizeof(display_name))) {
        return;
    }
    // 입력한 즐겨찾기 정보를 추가 요청으로 서버에 전달합니다.
    favorite_add(fd, type, target_value, display_name);
}

// 저장된 즐겨찾기 목록에서 번호로 선택한 항목을 삭제합니다.
static void run_favorite_delete_menu(socket_t fd, char *menu_input, unsigned long input_size) {
    int target_index;

    // 삭제 번호를 고를 수 있도록 현재 저장 목록을 먼저 출력합니다.
    favorite_list(fd);
    // 사용자가 삭제하려는 즐겨찾기 목록 번호를 입력받습니다.
    if (!read_line_prompt("삭제할 번호 입력: ", menu_input, input_size) ||
        !str_to_int(menu_input, &target_index) ||
        target_index <= 0) {
        printf("입력값을 다시 확인해주세요.\n");
        return;
    }
    // 선택한 목록 번호를 즐겨찾기 삭제 요청으로 서버에 전달합니다.
    favorite_delete(fd, target_index);
}

// 사용자가 종료를 선택할 때까지 메인 메뉴 루프를 유지합니다.
void menu_loop(socket_t fd) {
    char choice[16];
    char first_value[256];
    char second_value[256];

    // 0번 종료 입력이 들어오기 전까지 메뉴 선택을 반복합니다.
    for (;;) {
        print_menu();
        // 사용자의 메뉴 선택값을 문자열로 입력받습니다.
        if (!read_line_prompt("메뉴 선택: ", choice, sizeof(choice))) {
            break;
        }

        // 0번 입력은 클라이언트 메뉴 루프를 종료하는 값입니다.
        if (strcmp(choice, "0") == 0) {
            break;
        } else if (strcmp(choice, "1") == 0) {
            // 정류장 검색 요청 흐름을 실행합니다.
            run_single_keyword_menu(fd, PKT_STOP_SEARCH_REQ, "정류장명 또는 키워드 입력: ", first_value, sizeof(first_value));
        } else if (strcmp(choice, "2") == 0) {
            // 버스 번호 검색 요청 흐름을 실행합니다.
            run_single_keyword_menu(fd, PKT_BUS_SEARCH_REQ, "버스 번호 입력: ", first_value, sizeof(first_value));
        } else if (strcmp(choice, "3") == 0) {
            // 지하철역 검색 요청 흐름을 실행합니다.
            run_single_keyword_menu(fd, PKT_SUBWAY_SEARCH_REQ, "역명 또는 키워드 입력: ", first_value, sizeof(first_value));
        } else if (strcmp(choice, "4") == 0) {
            // 최단 경로 조회 요청 흐름을 실행합니다.
            run_route_search_menu(fd,
                                  "탑승 위치 입력: ",
                                  "도착 위치 입력: ",
                                  first_value,
                                  second_value,
                                  sizeof(first_value));
        } else if (strcmp(choice, "5") == 0) {
            // 주변 편의시설 조회 요청 흐름을 실행합니다.
            run_facility_menu(fd, first_value, second_value, sizeof(first_value));
        } else if (strcmp(choice, "6") == 0) {
            // 서버에 저장된 즐겨찾기 목록을 조회해 출력합니다.
            favorite_list(fd);
        } else if (strcmp(choice, "7") == 0) {
            // 즐겨찾기 추가 입력 흐름을 실행합니다.
            run_favorite_add_menu(fd);
        } else if (strcmp(choice, "8") == 0) {
            // 즐겨찾기 삭제 입력 흐름을 실행합니다.
            run_favorite_delete_menu(fd, first_value, sizeof(first_value));
        } else if (strcmp(choice, "9") == 0) {
            // 최근 검색 기록 조회 요청을 실행합니다.
            recent_list(fd);
        } else {
            printf("입력값을 다시 확인해주세요.\n");
        }
    }
}
