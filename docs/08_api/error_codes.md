# 오류 코드

## 공통 응답 코드

| code | 이름 | 의미 |
|------|------|------|
| 0 | OK | 정상 처리 |
| 1 | NOT_FOUND | 검색 결과 없음 |
| 2 | INVALID_REQUEST | 요청 형식 또는 입력값 오류 |
| 3 | DATA_LOAD_ERROR | 데이터 로딩 또는 데이터 참조 오류 |
| 4 | SERVER_ERROR | 서버 내부 오류 |

## 오류 응답

```text
ERROR|<code>:<message>
```

## 사용 예

```text
ERROR|1:NOT_FOUND
ERROR|2:INVALID_REQUEST
ERROR|3:DATA_LOAD_ERROR
ERROR|4:SERVER_ERROR
```

## 클라이언트 표시 원칙

| 코드 | 표시 문구 |
|------|-----------|
| 1 | 검색 결과가 없습니다. |
| 2 | 입력값을 다시 확인해주세요. |
| 3 | 데이터 파일을 확인해주세요. |
| 4 | 서버 처리 중 오류가 발생했습니다. |

