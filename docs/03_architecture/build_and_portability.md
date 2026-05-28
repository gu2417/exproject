# 빌드 및 이식성

## 1. 빌드 목표

| 대상 | 산출물 |
|------|--------|
| 서버 | `bin/transit_server` 또는 `bin/transit_server.exe` |
| 클라이언트 | `bin/transit_client` 또는 `bin/transit_client.exe` |

## 2. Makefile 요구사항

- `make` 실행 시 서버와 클라이언트를 모두 빌드한다.
- `make server`, `make client` 타깃을 제공할 수 있다.
- `make clean`은 빌드 산출물을 제거한다.
- Windows 빌드에서는 Winsock 라이브러리 링크를 포함한다.

## 3. 플랫폼 분기

| 기능 | Linux | Windows |
|------|-------|---------|
| socket header | `<sys/socket.h>` | `<winsock2.h>` |
| close | `close(fd)` | `closesocket(fd)` |
| init | 불필요 | `WSAStartup()` |
| cleanup | 불필요 | `WSACleanup()` |

공통 인터페이스는 `src/common/net_compat.h`에 둔다.

## 4. 경로 기준

실행 시 작업 디렉터리는 프로젝트 루트를 기본으로 한다.

```text
data/
client_data/
logs/
```

폴더가 없으면 생성하거나 명확한 오류를 출력한다.

