#include "client_net.h"

#include "protocol.h"
#include "result_viewer.h"

#include <stdio.h>
#include <string.h>

// 지정된 주소와 포트로 서버 연결을 시도합니다.
int client_connect(socket_t *out_fd, const char *host, int port) {
    struct sockaddr_in addr;
    socket_t fd;

    // 소켓 저장 위치와 서버 주소가 모두 있어야 접속을 시도합니다.
    if (!out_fd || !host) {
        return 0;
    }

    // 서버와 통신할 TCP 소켓을 생성합니다.
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == INVALID_SOCKET_VALUE) {
        return 0;
    }

    // 접속할 서버 주소 정보를 채웁니다.
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((unsigned short)port);
    addr.sin_addr.s_addr = inet_addr(host);

    // 서버에 연결을 시도합니다.
    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        // 실패하면 만든 소켓을 닫습니다.
        sock_close(fd);
        return 0;
    }

    // 성공한 소켓을 호출한 쪽에 넘깁니다.
    *out_fd = fd;
    return 1;
}

// 개행으로 끝나는 한 줄 요청을 서버에 전송합니다.
int client_send_line(socket_t fd, const char *line) {
    // 요청 문자열이 전달된 경우에만 서버 송신을 시작합니다.
    if (!line) {
        return 0;
    }
    // 문자열 전체가 보내졌는지 확인합니다.
    return send(fd, line, (int)strlen(line), 0) == (int)strlen(line);
}

// 서버가 보낸 한 줄 응답을 버퍼에 수신합니다.
int client_recv_line(socket_t fd, char *buf, unsigned long buf_size) {
    unsigned long pos = 0;
    // 응답을 담을 버퍼가 준비된 경우에만 수신을 진행합니다.
    if (!buf || buf_size == 0) {
        return 0;
    }
    // 프로토콜 응답의 끝을 나타내는 개행까지 바이트를 누적합니다.
    while (pos + 1 < buf_size) {
        char ch;
        int received = recv(fd, &ch, 1, 0);
        // 수신 도중 연결 종료나 오류가 발생하면 읽기 실패로 반환합니다.
        if (received <= 0) {
            return 0;
        }
        // 읽은 글자를 버퍼에 저장합니다.
        buf[pos++] = ch;
        // 한 줄이 끝나면 읽기를 멈춥니다.
        if (ch == '\n') {
            break;
        }
    }
    // C 문자열 끝을 표시합니다.
    buf[pos] = '\0';
    return pos > 0;
}

// 요청을 보내고 받은 결과를 화면에 출력합니다.
int client_send_request(socket_t fd, const char *request) {
    char response[MAX_PACKET_SIZE];

    // 요청 전송 결과를 확인하고, 실패 시 사용자에게 오류를 알린 뒤 반환합니다.
    if (!client_send_line(fd, request)) {
        printf("서버로 요청을 보낼 수 없습니다.\n");
        return 0;
    }
    // 서버 응답을 받지 못하면 오류 메시지를 출력하고 호출 흐름으로 돌아갑니다.
    if (!client_recv_line(fd, response, sizeof(response))) {
        printf("서버 응답을 받을 수 없습니다.\n");
        return 0;
    }
    // 받은 응답을 종류에 맞게 출력합니다.
    result_viewer_show(response);
    // 정상 응답이면 1, 아니면 0을 돌려줍니다.
    return response_code(response) == ERR_OK;
}
