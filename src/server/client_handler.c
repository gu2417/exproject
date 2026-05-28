#include "client_handler.h"

#include "log.h"
#include "protocol.h"
#include "router.h"
#include "string_utils.h"

#include <stdio.h>
#include <string.h>

// 클라이언트 소켓에서 요청을 읽고 처리 결과를 다시 전송합니다.
void handle_client(socket_t client_fd, const char *client_ip, AppData *data) {
    char request[MAX_PACKET_SIZE];
    char response[MAX_PACKET_SIZE];
    int pos = 0;

    // 클라이언트 접속을 로그에 남깁니다.
    log_info(client_ip, "CONNECT", ERR_OK, "client connected");

    // 연결이 유지되는 동안 줄 단위 요청을 반복해서 수신합니다.
    for (;;) {
        char ch;
        // 패킷 종료 문자인 개행을 만날 때까지 바이트 단위로 수신합니다.
        int received = recv(client_fd, &ch, 1, 0);
        // 읽지 못하면 연결이 끊긴 것으로 보고 반복을 끝냅니다.
        if (received <= 0) {
            break;
        }
        // 패킷이 너무 길면 오류 응답을 보내고 버퍼를 비웁니다.
        if (pos + 1 >= MAX_PACKET_SIZE) {
            snprintf(response, sizeof(response), "%s|%d:INVALID_REQUEST\n", PKT_ERROR, ERR_INVALID_REQUEST);
            send(client_fd, response, (int)strlen(response), 0);
            pos = 0;
            continue;
        }
        // 읽은 글자를 요청 버퍼에 넣습니다.
        request[pos++] = ch;
        // 줄바꿈을 만나면 요청 하나가 끝난 것입니다.
        if (ch == '\n') {
            int result;
            // 문자열 끝을 표시하고 응답 버퍼를 비웁니다.
            request[pos] = '\0';
            response[0] = '\0';
            // 요청 종류에 맞는 기능을 실행합니다.
            result = route_request(data, request, response, sizeof(response));
            // 처리 결과를 로그로 남깁니다.
            log_info(client_ip, request, result, "request processed");
            // 처리 함수가 만든 응답 문자열을 클라이언트 소켓으로 전송합니다.
            if (response[0]) {
                send(client_fd, response, (int)strlen(response), 0);
            }
            // 다음 요청을 받기 위해 위치를 처음으로 돌립니다.
            pos = 0;
        }
    }

    // 접속 종료를 로그에 남기고 소켓을 닫습니다.
    log_info(client_ip, "DISCONNECT", ERR_OK, "client disconnected");
    sock_close(client_fd);
}
