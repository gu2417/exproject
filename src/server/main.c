#include "file_utils.h"
#include "data_loader.h"
#include "linked_list.h"
#include "log.h"
#include "net_compat.h"
#include "server.h"
#include "types.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

// 서버 프로그램의 시작 지점입니다.
int main(int argc, char **argv) {
    ServerContext ctx;
    int port = 8080;

    // 실행 인자로 포트가 들어오면 그 값을 사용합니다.
    if (argc >= 2) {
        port = atoi(argv[1]);
        // 잘못된 포트 값이면 기본 포트로 돌립니다.
        if (port <= 0) {
            port = 8080;
        }
    }

#ifdef _WIN32
    // 윈도우 콘솔에서 한글 출력이 깨지지 않게 맞춥니다.
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
#endif

    // 서버 상태 구조체를 비운 뒤 포트와 데이터 저장소를 준비합니다.
    memset(&ctx, 0, sizeof(ctx));
    ctx.port = port;
    app_data_init(&ctx.data);

    // 로그 폴더와 로그 파일을 준비합니다.
    ensure_dir("logs");
    if (!log_init("logs/server.log")) {
        fprintf(stderr, "failed to open logs/server.log\n");
    }

    // 네트워크 사용 준비를 합니다.
    if (!net_init()) {
        fprintf(stderr, "network init failed: %s\n", net_last_error());
        log_close();
        return 1;
    }

    // 데이터와 사용자 저장 파일 폴더를 준비합니다.
    ensure_dir("data");
    ensure_dir("client_data");
    // CSV 파일들을 읽어 메모리에 올립니다.
    if (!data_loader_load(&ctx.data)) {
        fprintf(stderr, "data load failed. check data/*.csv and logs/server.log\n");
        app_data_free(&ctx.data);
        net_cleanup();
        log_close();
        return 1;
    }

    // 서버 소켓을 열고 클라이언트 요청을 받습니다.
    server_run(&ctx);

    // 서버 종료 시 메모리와 네트워크를 정리합니다.
    app_data_free(&ctx.data);
    net_cleanup();
    log_close();
    return 0;
}
