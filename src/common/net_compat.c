#include "net_compat.h"

#include <stdio.h>
#include <string.h>

#ifdef _WIN32
// 윈도우 소켓 사용을 시작합니다.
int net_init(void) {
    WSADATA wsa;
    return WSAStartup(MAKEWORD(2, 2), &wsa) == 0;
}

// 윈도우 소켓 사용을 끝냅니다.
void net_cleanup(void) {
    WSACleanup();
}

// 윈도우 방식으로 소켓을 닫습니다.
void sock_close(socket_t fd) {
    if (fd != INVALID_SOCKET_VALUE) {
        closesocket(fd);
    }
}

// 마지막 Windows 소켓 오류 코드를 사람이 읽을 수 있는 문자열로 변환합니다.
const char *net_last_error(void) {
    static char buf[64];
    snprintf(buf, sizeof(buf), "winsock error %d", WSAGetLastError());
    return buf;
}
#else
#include <errno.h>

// 리눅스 계열에서는 따로 시작 작업이 필요 없습니다.
int net_init(void) {
    return 1;
}

// 리눅스 계열에서는 따로 정리 작업이 필요 없습니다.
void net_cleanup(void) {
}

// 일반 close 함수로 소켓을 닫습니다.
void sock_close(socket_t fd) {
    if (fd != INVALID_SOCKET_VALUE) {
        close(fd);
    }
}

// 마지막 오류 번호를 문자열로 바꿉니다.
const char *net_last_error(void) {
    return strerror(errno);
}
#endif
