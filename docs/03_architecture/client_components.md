# 클라이언트 구성 요소

## 1. 컴포넌트 목록

| 컴포넌트 | 주요 파일 | 책임 |
|----------|-----------|------|
| Client Main | `src/client/main.c` | 서버 접속 정보 입력, 메뉴 루프 시작 |
| Menu | `src/client/menu.c/h` | 콘솔 메뉴 출력, 메뉴 번호 검증 |
| Client Net | `src/client/client_net.c/h` | TCP connect, send, recv |
| Request Builder | `src/client/request_builder.c/h` | 사용자 입력을 패킷 문자열로 변환 |
| Result Viewer | `src/client/result_viewer.c/h` | 서버 응답을 사용자 친화적으로 출력 |
| Favorite | `src/client/favorite.c/h` | 즐겨찾기 추가/조회/삭제/중복 검사 |
| Recent Search | `src/client/recent_search.c/h` | 최근 검색 기록 저장/조회 |

## 2. 메뉴 처리 흐름

```mermaid
flowchart TD
    A[메뉴 출력] --> B{번호 입력}
    B -->|1~5| C[검색/조회 입력 받기]
    B -->|6| D[즐겨찾기 조회]
    B -->|7| E[즐겨찾기 추가]
    B -->|8| K[즐겨찾기 삭제]
    B -->|9| F[최근 검색 기록 조회]
    B -->|0| Z[종료]
    C --> G[요청 패킷 생성]
    G --> H[서버 전송]
    H --> I[응답 수신]
    I --> J[결과 출력]
    J --> A
    D --> A
    E --> A
    K --> A
    F --> A
```

## 3. 로컬 파일 책임

| 파일 | 처리 위치 | 비고 |
|------|-----------|------|
| `client_data/favorites.txt` | 클라이언트 | 서버에 전송하지 않는 로컬 편의 기능 |
| `client_data/recent_search.txt` | 클라이언트 | 검색 요청 전후에 기록 |

즐겨찾기와 최근 검색 기록은 서버 세션과 독립적으로 동작한다.
