#ifndef TRANSIT_NET_COMPAT_H
#define TRANSIT_NET_COMPAT_H

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
typedef SOCKET socket_t;
#define INVALID_SOCKET_VALUE INVALID_SOCKET
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
typedef int socket_t;
#define INVALID_SOCKET_VALUE (-1)
#endif

// 운영체제별 네트워크 시작과 정리를 감쌉니다.
int net_init(void);
void net_cleanup(void);
// 운영체제에 맞게 소켓을 닫고 오류 문자열을 가져옵니다.
void sock_close(socket_t fd);
const char *net_last_error(void);

#endif
