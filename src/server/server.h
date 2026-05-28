#ifndef TRANSIT_SERVER_H
#define TRANSIT_SERVER_H

#include "types.h"

// 서버 소켓을 열고 접속을 기다립니다.
int server_run(ServerContext *ctx);

#endif
