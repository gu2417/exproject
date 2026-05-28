# 공통 모듈

| 파일 | 책임 |
|------|------|
| `protocol.h` | 패킷 타입, 구분자, 오류 코드, 최대 길이 상수 |
| `types.h` | 정류장, 노선, 역, 편의시설, 그래프, 세션 구조체 |
| `linked_list.c/h` | 연결리스트 삽입, 검색, 순회, 해제 |
| `graph.c/h` | 그래프 노드/간선 관리, 경로 탐색 |
| `csv_parser.c/h` | CSV 라인 파싱, 컬럼 추출 |
| `string_utils.c/h` | trim, safe copy, keyword match, 숫자 검증 |
| `net_compat.h` | Windows/Linux 소켓 차이 추상화 |

## 공통 상수 예시

```c
#define MAX_PACKET_SIZE 4096
#define FIELD_SEP '|'
#define ITEM_SEP ':'
#define LIST_SEP ';'
#define PACKET_TERM '\n'
```

공통 모듈은 서버와 클라이언트가 같은 규칙을 사용하도록 유지하는 기준점이다.

