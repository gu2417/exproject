# 연결 확인 패킷

## HELLO

```text
C->S  HELLO|<client_name>
S->C  HELLO_RES|OK:<server_version>
```

| 필드 | 설명 |
|------|------|
| `client_name` | 클라이언트 식별용 표시명 |
| `server_version` | 서버 버전 문자열 |

## PING

```text
C->S  PING|
S->C  PONG|
```

연결 상태 확인용 패킷이다. 필수 구현이 아니더라도 디버깅에 유용하다.

