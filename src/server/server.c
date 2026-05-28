#include "server.h"

#include "client_handler.h"
#include "log.h"
#include "net_compat.h"

#include <stdio.h>
#include <string.h>

// 서버를 열고 클라이언트 접속을 기다립니다.
int server_run(ServerContext *ctx) {
    struct sockaddr_in addr;
    socket_t listen_fd;
    int opt = 1;

    // 서버 상태 구조체가 준비되지 않으면 소켓 초기화를 진행하지 않습니다.
    if (!ctx) {
        return 0;
    }

    // 클라이언트 연결을 받을 서버용 TCP 소켓을 생성합니다.
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == INVALID_SOCKET_VALUE) {
        log_error("-", "SERVER_START", 4, net_last_error());
        return 0;
    }

    // 서버를 바로 다시 실행할 때 포트를 다시 쓸 수 있게 합니다.
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt));

    // 모든 주소에서 지정 포트로 접속을 받도록 설정합니다.
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons((unsigned short)ctx->port);

    // 소켓에 주소와 포트를 묶습니다.
    if (bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        log_error("-", "BIND", 4, net_last_error());
        sock_close(listen_fd);
        return 0;
    }
    // 접속 대기 상태로 바꿉니다.
    if (listen(listen_fd, 8) != 0) {
        log_error("-", "LISTEN", 4, net_last_error());
        sock_close(listen_fd);
        return 0;
    }

    // 열린 서버 소켓 번호를 저장하고 시작 로그를 남깁니다.
    ctx->listen_fd = (int)listen_fd;
    log_info("-", "SERVER_START", 0, "server listening");
    printf("Server listening on port %d\n", ctx->port);

    // 서버가 종료될 때까지 접속을 계속 받습니다.
    for (;;) {
        struct sockaddr_in client_addr;
        socklen_t len = sizeof(client_addr);
        char client_ip[64];
        // 클라이언트 접속을 하나 받습니다.
        socket_t client_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &len);
        // 접속 받기에 실패하면 로그만 남기고 다음 접속을 기다립니다.
        if (client_fd == INVALID_SOCKET_VALUE) {
            log_error("-", "ACCEPT", 4, net_last_error());
            continue;
        }
        // 접속한 클라이언트 IP를 문자열로 저장합니다.
        snprintf(client_ip, sizeof(client_ip), "%s", inet_ntoa(client_addr.sin_addr));
        // 수락된 클라이언트 소켓을 요청 처리 루틴으로 넘깁니다.
        handle_client(client_fd, client_ip, &ctx->data);
    }
}
