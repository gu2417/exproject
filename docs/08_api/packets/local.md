# 로컬 기능 패킷

즐겨찾기와 최근 검색 기록은 서버에 전송하지 않는 클라이언트 로컬 기능이다.
문서상 형식은 구현 일관성을 위해 정의한다.

## 즐겨찾기

```text
LOCAL  FAVORITE_ADD|<type>:<id>:<name>
LOCAL  FAVORITE_LIST|
LOCAL  FAVORITE_DELETE|<index>
```

## 최근 검색

```text
LOCAL  RECENT_ADD|<type>:<keyword>:<timestamp>
LOCAL  RECENT_LIST|
```

## 저장 파일

| 기능 | 파일 |
|------|------|
| 즐겨찾기 | `client_data/favorites.txt` |
| 최근 검색 | `client_data/recent_search.txt` |
