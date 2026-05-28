# recent_search.txt

클라이언트 로컬 최근 검색 기록 저장 파일이다.

## 형식

```text
<type>|<keyword>|<timestamp>
```

| 필드 | 설명 |
|------|------|
| `type` | `stop`, `bus`, `station`, `route`, `transfer`, `facility` |
| `keyword` | 사용자가 입력한 검색어 또는 요약 |
| `timestamp` | `YYYY-MM-DD HH:MM:SS` |

## 규칙

- 최신순 출력은 파일 전체를 읽은 뒤 역순 표시로 구현할 수 있다.
- 최근 N개만 보여주는 제한을 둘 수 있다.
- 손상된 행은 건너뛰고 로그 또는 안내 없이 계속 진행한다.
