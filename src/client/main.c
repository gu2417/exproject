#include "client_net.h"
#include "file_utils.h"
#include "menu.h"
#include "net_compat.h"
#include "protocol.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

// 클라이언트 프로그램의 시작 지점입니다.
int main(int argc, char **argv) {
    // 기본 접속 주소와 포트는 로컬 서버로 둡니다.
    const char *host = "127.0.0.1";
    int port = 8080;
    socket_t fd;
    char response[MAX_PACKET_SIZE];

    // 첫 번째 실행 인자가 있으면 서버 주소로 사용합니다.
    if (argc >= 2) {
        host = argv[1];
    }
    // 두 번째 실행 인자가 있으면 포트 번호로 사용합니다.
    if (argc >= 3) {
        port = atoi(argv[2]);
        // 잘못된 포트 값이면 기본 포트로 되돌립니다.
        if (port <= 0) {
            port = 8080;
        }
    }

#ifdef _WIN32
    // 윈도우 콘솔에서 한글이 깨지지 않게 UTF-8로 맞춥니다.
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
#endif

    // 클라이언트가 쓸 파일 폴더를 준비합니다.
    ensure_dir("client_data");

    // 소켓 사용 준비를 합니다.
    if (!net_init()) {
        fprintf(stderr, "network init failed: %s\n", net_last_error());
        return 1;
    }

    // 지정된 주소와 포트로 서버 연결을 시도합니다.
    if (!client_connect(&fd, host, port)) {
        fprintf(stderr, "server connection failed: %s:%d\n", host, port);
        net_cleanup();
        return 1;
    }

    // 처음 접속했음을 서버에 알립니다.
    client_send_line(fd, "HELLO|console_client\n");
    // 서버 인사 응답이 오면 화면에 보여줍니다.
    if (client_recv_line(fd, response, sizeof(response))) {
        printf("%s", response);
    }

    // 실제 메뉴 입력 처리를 시작합니다.
    menu_loop(fd);

    // 사용이 끝난 소켓과 네트워크를 정리합니다.
    sock_close(fd);
    net_cleanup();
    return 0;
}
