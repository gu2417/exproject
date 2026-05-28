# 네이밍 규칙

## 1. 파일명

| 유형 | 규칙 | 예 |
|------|------|----|
| C source | snake_case | `search_service.c` |
| Header | snake_case | `search_service.h` |
| CSV | snake_case | `bus_stops.csv` |
| Markdown | snake_case | `packet_format.md` |

## 2. 함수명

| 범위 | 접두어 | 예 |
|------|--------|----|
| 서버 | `server_`, `handle_` | `handle_stop_search()` |
| 클라이언트 | `client_`, `menu_` | `menu_show_main()` |
| 리스트 | `list_` | `list_find_stop_by_keyword()` |
| 그래프 | `graph_` | `graph_find_path()` |
| 문자열 | `str_` | `str_trim()` |

## 3. 상수와 매크로

- 패킷 타입: `PKT_STOP_SEARCH_REQ`
- 오류 코드: `ERR_NOT_FOUND`
- 최대 길이: `MAX_STOP_NAME_LEN`
- enum 값: 대문자 snake_case

## 4. 구조체

구조체 타입은 PascalCase를 사용한다.

```c
typedef struct StopNode StopNode;
typedef struct TransferGraph TransferGraph;
typedef struct ServerContext ServerContext;
```

