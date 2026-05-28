# 운행 정보 표시(FR-O01 ~ O04)

## FR-O01 운행 상태 표시 - P4

| 항목 | 내용 |
|------|------|
| 입력 | 노드 ID, 노선 번호, 시간대 |
| 데이터 | `arrival_test.csv` |
| 상태값 | `운행 시작`, `곧 도착`, `운행 종료` |
| 표시 위치 | 정류장 검색 결과 또는 운행 정보 상세 출력 |

## FR-O02 도착 예정시간 표시 - P4

| 항목 | 내용 |
|------|------|
| 입력 | 정류장/역 ID, 노선 번호, 시간대 |
| 패킷 | `ARRIVAL_REQ` / `ARRIVAL_RES` |
| 출력 | 노선 번호, 노드명, 도착 예정 분, 운행 상태 |
| 예외 | 테스트 데이터가 없으면 `NOT_FOUND` |

## FR-O03 혼잡도 표시 - P4

| 항목 | 내용 |
|------|------|
| 입력 | 노드 ID, 노선 번호, 시간대 |
| 패킷 | `CROWDING_REQ` / `CROWDING_RES` |
| 데이터 | `crowding_test.csv` |
| 출력 | `여유`, `보통`, `혼잡` |

## FR-O04 시간대 기반 계산 - P4

| 항목 | 내용 |
|------|------|
| 기본값 | 현재 시간대 |
| 테스트 | 사용자가 시연용 시간대를 입력할 수 있음 |
| 목적 | 실제 API 없이도 운행 정보 화면을 검증 가능하게 함 |
| 주의 | 실시간 정확도를 보장하지 않음 |

## 관련 문서

- 패킷: [`08_api/packets/operation.md`](../08_api/packets/operation.md)
- 데이터 스키마: [`07_database/tables/arrival_test.md`](../07_database/tables/arrival_test.md), [`07_database/tables/crowding_test.md`](../07_database/tables/crowding_test.md)

