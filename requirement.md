# Requirements Specification
# C Socket Public Transit Information System - Asan Bus/Subway Route Guide

**Version**: 1.0.0  
**Date**: 2026-05-21  
**Language**: C (C11)  
**Architecture**: TCP Server-Client, CSV File I/O, Linked List + Graph

---

## 1. 프로젝트 개요

아산시 버스 정류장, 버스 노선, 지하철역, 주변 편의시설 정보를 조회할 수 있는 대중교통 정보 시스템을 구현한다.  
프로그램은 서버/클라이언트 구조로 동작하며, 클라이언트는 콘솔 메뉴를 통해 검색 요청을 보내고 서버는 CSV 파일에서 로드한 교통 데이터를 기반으로 결과를 반환한다.  
프로젝트 주제는 채팅 및 학생정보관리가 아니며, 공공데이터포털에서 다운로드한 CSV 데이터를 파일 입출력으로 읽어 사용한다.

본 프로젝트는 필수 기술 요구사항에 따라 소켓 프로그래밍, 연결리스트, 그래프 자료구조, 파일 입출력, 서버 로그 저장 기능을 포함한다.

---

## 2. 시스템 아키텍처

```text
+----------------------------------------------------------+
|                         SERVER                           |
|  +------------+   +---------------+   +----------------+ |
|  | Accept     |   | Client        |   | Command        | |
|  | Loop       |-> | Handler       |-> | Router         | |
|  +------------+   +---------------+   +----------------+ |
|                              |                           |
|                              v                           |
|  +----------------------------------------------------+  |
|  | Search Service                                      |  |
|  | stop search | bus route search | subway search      |  |
|  | transfer path | facility recommendation             |  |
|  | ETA/status/crowding calculation                     |  |
|  +-------------------------+--------------------------+  |
|                            |                             |
|  +-------------------------v--------------------------+  |
|  | In-Memory Data Store                                |  |
|  | linked lists: stops, routes, facilities             |  |
|  | graph: transfer network                            |  |
|  +-------------------------+--------------------------+  |
|                            |                             |
|  +-------------------------v--------------------------+  |
|  | CSV Loader / File Writer                            |  |
|  | data/*.csv | logs/server.log                        |  |
|  +----------------------------------------------------+  |
+----------------------------------------------------------+
                  ^ TCP Socket ^
                  |            |
+----------------------------------------------------------+
|                         CLIENT                           |
|  +----------------+   +----------------+                 |
|  | Console Menu   |-> | Request Builder |                |
|  +----------------+   +----------------+                 |
|           ^                    |                          |
|           |                    v                          |
|  +----------------+   +----------------+                 |
|  | Result Viewer  |<- | Socket Client   |                 |
|  +----------------+   +----------------+                 |
|           |                                               |
|           v                                               |
|  recent_search.txt | favorites.txt                       |
+----------------------------------------------------------+
```

### 2-1. 데이터 유형 구분

| 데이터 유형 | 사용 목적 | 저장/처리 방식 |
|-------------|-----------|----------------|
| 버스 정류장 | 정류장 검색, 정류장별 경유 버스 조회 | CSV 로드 후 연결리스트 저장 |
| 버스 노선 | 버스 번호 검색, 노선도 조회 | CSV 로드 후 노선별 연결리스트 저장 |
| 지하철역 | 지하철역 검색, 환승 경로 구성 | CSV 로드 후 연결리스트 및 그래프 노드 저장 |
| 편의시설 | 주변 편의시설 추천 | CSV 로드 후 연결리스트 저장 |
| 도착 예정시간 | 도착 예정시간 표시 | 원본 데이터 가공 및 테스트용 CSV 추가 |
| 혼잡도 | 시간대별 혼잡도 표시 | API 연동 없이 시간대별 테스트 데이터 사용 |
| 즐겨찾기 | 자주 찾는 정류장/노선/역 저장 | 클라이언트 로컬 텍스트 파일 저장 |
| 최근 검색 기록 | 최근 검색 메뉴 제공 | 클라이언트 로컬 텍스트 파일 저장 |
| 서버 로그 | 요청/응답/오류 추적 | 서버 로그 파일 저장 및 콘솔 출력 |

---

## 3. 기능 요구사항

### 3-1. 데이터 로딩 및 자료구조

| ID | 기능 | 설명 |
|----|------|------|
| FR-DATA01 | CSV 파일 로드 | 공공데이터포털에서 받은 버스 정류장, 버스 노선, 지하철역, 편의시설 CSV 파일을 프로그램 시작 시 읽는다 |
| FR-DATA02 | 연결리스트 저장 | 정류장, 노선, 지하철역, 편의시설 데이터를 연결리스트 기반 구조에 저장한다 |
| FR-DATA03 | 그래프 생성 | 정류장/역을 노드로, 이동 가능 구간을 간선으로 구성하여 환승 경로 탐색에 사용한다 |
| FR-DATA04 | 데이터 검증 | 필수 컬럼 누락, 빈 ID, 중복 ID, 잘못된 노선 참조를 로딩 시 점검한다 |
| FR-DATA05 | 테스트 데이터 추가 | 도착 예정시간과 혼잡도는 원본 데이터 부족 시 별도 테스트 CSV로 보완한다 |

---

### 3-2. 교통 정보 검색

| ID | 기능 | 설명 |
|----|------|------|
| FR-S01 | 정류장 검색 | 정류장명 또는 키워드로 정류장을 검색하고, 해당 정류장에 정차하는 버스 목록을 표시한다 |
| FR-S02 | 버스 번호 검색 | 버스 번호를 입력하면 출발지, 종점, 경유 정류장 순서, 운행 방향을 표시한다 |
| FR-S03 | 지하철역 검색 | 지하철역명 또는 키워드로 역 정보를 검색하고, 노선 및 환승 가능 여부를 표시한다 |
| FR-S04 | 환승 정보 조회 | 출발 정류장/역과 도착 정류장/역을 입력하면 그래프 탐색으로 환승 경로를 제공한다 |
| FR-S05 | 주변 편의시설 조회 | 선택한 정류장 또는 지하철역 주변의 편의시설을 카테고리별로 추천한다 |
| FR-S06 | 검색 결과 없음 처리 | 검색 결과가 없을 때 명확한 안내 메시지를 반환한다 |

---

### 3-3. 운행 정보 표시

| ID | 기능 | 설명 |
|----|------|------|
| FR-O01 | 운행 상태 표시 | 버스 또는 지하철의 상태를 `운행 시작`, `곧 도착`, `운행 종료` 중 하나로 표시한다 |
| FR-O02 | 도착 예정시간 표시 | 정류장/역과 노선 정보를 기반으로 예상 도착 시간을 분 단위로 표시한다 |
| FR-O03 | 혼잡도 표시 | 시간대별 테스트 데이터를 사용하여 `여유`, `보통`, `혼잡` 등급을 표시한다 |
| FR-O04 | 시간대 기반 계산 | 현재 시간 또는 사용자가 입력한 테스트 시간대에 따라 도착 예정시간과 혼잡도를 산출한다 |

---

### 3-4. 사용자 편의 기능

| ID | 기능 | 설명 |
|----|------|------|
| FR-U01 | 즐겨찾기 추가 | 정류장, 버스 번호, 지하철역을 즐겨찾기에 추가한다 |
| FR-U02 | 즐겨찾기 조회 | 저장된 즐겨찾기 목록을 메뉴에서 조회한다 |
| FR-U03 | 즐겨찾기 중복 방지 | 동일 항목이 중복 저장되지 않도록 처리한다 |
| FR-U04 | 최근 검색 기록 저장 | 최근 검색한 키워드와 검색 유형을 `recent_search.txt`에 저장한다 |
| FR-U05 | 최근 검색 기록 조회 | 이전 검색 내용을 확인할 수 있는 메뉴 UI를 제공한다 |
| FR-U06 | 메뉴 기반 UI | 최종 메뉴 구성에 맞춘 콘솔 인터페이스를 제공한다 |

---

### 3-5. 서버/클라이언트 및 로그

| ID | 기능 | 설명 |
|----|------|------|
| FR-N01 | 서버 실행 | 서버는 지정 포트에서 클라이언트 접속을 대기한다 |
| FR-N02 | 클라이언트 접속 | 클라이언트는 서버 IP와 포트로 접속하여 요청을 전송한다 |
| FR-N03 | 요청 라우팅 | 서버는 요청 타입에 따라 검색, 환승, 편의시설, 운행 정보 기능으로 분기한다 |
| FR-N04 | 서버 로그 저장 | 클라이언트 접속, 요청 타입, 처리 결과, 오류를 `logs/server.log`에 저장하고 서버 화면에도 표시한다 |
| FR-N05 | 연결 종료 처리 | 클라이언트 종료 또는 네트워크 오류 시 서버가 안정적으로 세션을 정리한다 |

---

### 3-6. 범위 제외 항목 *(Out-of-Scope)*

> 아래 기능은 현재 프로젝트 범위에서 제외한다.

| ID | 기능 | 설명 |
|----|------|------|
| FR-X01 | ~~채팅 기능~~ | ~~프로젝트 주제 제한에 따라 구현하지 않는다~~ |
| FR-X02 | ~~학생정보관리~~ | ~~프로젝트 주제 제한에 따라 구현하지 않는다~~ |
| FR-X03 | ~~실시간 API 혼잡도 연동~~ | ~~API 연동 없이 시간대별 테스트 데이터로 구현한다~~ |
| FR-X04 | ~~정확한 실시간 위치 추적~~ | ~~정적 CSV 및 테스트 데이터를 기반으로 안내한다~~ |
| FR-X05 | ~~GUI 애플리케이션~~ | ~~현재 범위는 콘솔 메뉴 기반 클라이언트로 한다~~ |

---

## 4. 메시지 프로토콜

### 4-1. 패킷 형식

```text
<TYPE>|<PAYLOAD>\n
```

- 필드 구분자: `|`
- 항목 내부 구분자: `:`
- 리스트 항목 구분자: `;`
- 종단 문자: `\n`
- 최대 패킷 길이: 4096 bytes
- 문자열 인코딩: UTF-8

### 4-2. 패킷 정의

**연결 확인**

```text
C->S  HELLO|<client_name>
S->C  HELLO_RES|OK:<server_version>

C->S  PING|
S->C  PONG|
```

**정류장/노선/지하철역 검색**

```text
C->S  STOP_SEARCH_REQ|<keyword>
S->C  STOP_SEARCH_RES|<code>|<stop_id>:<stop_name>:<route_no,route_no,...>;<stop_id>:...

C->S  BUS_SEARCH_REQ|<route_no>
S->C  BUS_SEARCH_RES|<code>|<route_no>:<start_stop>:<end_stop>:<stop1,stop2,stop3,...>

C->S  SUBWAY_SEARCH_REQ|<keyword>
S->C  SUBWAY_SEARCH_RES|<code>|<station_id>:<station_name>:<line_no>:<transfer_lines>;<station_id>:...
```

**환승 및 편의시설**

```text
C->S  TRANSFER_REQ|<from_node_keyword>:<to_node_keyword>
S->C  TRANSFER_RES|<code>|<path_node1,path_node2,...>:<transfer_count>:<estimated_minutes>

C->S  FACILITY_REQ|<node_keyword>:<category>
S->C  FACILITY_RES|<code>|<facility_name>:<category>:<nearest_node>:<address>;<facility_name>:...
```

**운행 정보**

```text
C->S  ARRIVAL_REQ|<node_id>:<route_no>:<time_slot>
S->C  ARRIVAL_RES|<code>|<route_no>:<node_name>:<arrival_minutes>:<status>

C->S  CROWDING_REQ|<node_id>:<route_no>:<time_slot>
S->C  CROWDING_RES|<code>|<route_no>:<node_name>:<time_slot>:<crowding_level>
```

**즐겨찾기/최근 검색**

```text
# 즐겨찾기와 최근 검색 기록은 클라이언트 로컬 파일로 관리한다.
# 서버에는 실제 검색 요청만 전송한다.
LOCAL  FAVORITE_ADD|<type>:<id>:<name>
LOCAL  FAVORITE_LIST|
LOCAL  RECENT_ADD|<type>:<keyword>:<timestamp>
LOCAL  RECENT_LIST|
```

**오류 응답**

```text
S->C  ERROR|<code>:<message>
```

| code | 의미 |
|------|------|
| 0 | OK |
| 1 | NOT_FOUND |
| 2 | INVALID_REQUEST |
| 3 | DATA_LOAD_ERROR |
| 4 | SERVER_ERROR |

---

## 5. 파일 기반 데이터 스키마

### 5-1. CSV 입력 파일

| 파일 | 주요 컬럼 | 설명 |
|------|-----------|------|
| `data/bus_stops.csv` | `stop_id, stop_name, lat, lng, district` | 아산시 버스 정류장 목록 |
| `data/bus_routes.csv` | `route_no, route_name, start_stop_id, end_stop_id, interval_min` | 버스 노선 기본 정보 |
| `data/route_stops.csv` | `route_no, seq, stop_id, direction` | 노선별 정류장 순서 |
| `data/subway_stations.csv` | `station_id, station_name, line_no, lat, lng, transfer_lines` | 지하철역 정보 |
| `data/facilities.csv` | `facility_id, name, category, nearest_node_id, address, lat, lng` | 정류장/역 주변 편의시설 |
| `data/arrival_test.csv` | `route_no, node_id, time_slot, arrival_minutes, status` | 도착 예정시간 및 운행 상태 테스트 데이터 |
| `data/crowding_test.csv` | `route_no, node_id, time_slot, crowding_level` | 시간대별 혼잡도 테스트 데이터 |

### 5-2. 출력 및 저장 파일

| 파일 | 설명 |
|------|------|
| `logs/server.log` | 서버 접속, 요청, 응답, 오류 로그 |
| `client_data/favorites.txt` | 클라이언트 즐겨찾기 저장 파일 |
| `client_data/recent_search.txt` | 클라이언트 최근 검색 기록 저장 파일 |

---

## 6. 자료구조 설계

### 6-1. 연결리스트 기반 데이터

```c
typedef struct StopNode {
    char stop_id[32];
    char stop_name[100];
    double lat;
    double lng;
    char district[50];
    struct StopNode *next;
} StopNode;

typedef struct RouteStopNode {
    char route_no[32];
    int seq;
    char stop_id[32];
    char direction[50];
    struct RouteStopNode *next;
} RouteStopNode;

typedef struct RouteNode {
    char route_no[32];
    char route_name[100];
    char start_stop_id[32];
    char end_stop_id[32];
    int interval_min;
    RouteStopNode *stops;
    struct RouteNode *next;
} RouteNode;

typedef struct FacilityNode {
    char facility_id[32];
    char name[100];
    char category[50];
    char nearest_node_id[32];
    char address[200];
    struct FacilityNode *next;
} FacilityNode;
```

### 6-2. 그래프 기반 환승 네트워크

```c
typedef struct GraphEdge {
    int to_index;
    int weight_min;
    char route_no[32];
    struct GraphEdge *next;
} GraphEdge;

typedef struct GraphNode {
    char node_id[32];
    char node_name[100];
    char node_type[16];   /* bus_stop | subway_station */
    GraphEdge *edges;
} GraphNode;

typedef struct TransferGraph {
    GraphNode *nodes;
    int node_count;
    int capacity;
} TransferGraph;
```

### 6-3. 서버 애플리케이션 데이터

```c
typedef struct AppData {
    StopNode *stops;
    RouteNode *routes;
    FacilityNode *facilities;
    TransferGraph graph;
} AppData;
```

---

## 7. 서버 인메모리 세션 구조체

```c
typedef struct ClientSession {
    int fd;
    char client_ip[64];
    int active;
} ClientSession;

typedef struct ServerContext {
    int listen_fd;
    int port;
    AppData data;
    ClientSession sessions[32];
} ServerContext;
```

서버는 클라이언트별 요청을 처리하면서 공통 데이터는 읽기 중심으로 사용한다. 로그 파일 기록은 동시에 여러 요청이 들어올 수 있으므로 mutex 또는 단일 로그 함수로 보호한다.

---

## 8. 클라이언트 콘솔 메뉴 설계

### 메인 메뉴

```text
===== 아산시 대중교통 정보 시스템 =====

1. 정류장 검색
2. 버스 번호 검색
3. 지하철역 검색
4. 환승 정보 조회
5. 주변 편의시설 조회
6. 즐겨찾기 조회
7. 즐겨찾기 추가
8. 최근 검색 기록
0. 종료
```

### 메뉴별 입력/출력

| 메뉴 | 입력 | 출력 |
|------|------|------|
| 정류장 검색 | 정류장명 또는 키워드 | 정류장 목록, 경유 버스, 도착 예정시간 |
| 버스 번호 검색 | 버스 번호 | 출발지, 종점, 전체 경유 정류장 |
| 지하철역 검색 | 역명 또는 키워드 | 역명, 노선, 환승 가능 노선 |
| 환승 정보 조회 | 출발지, 도착지 | 추천 경로, 환승 횟수, 예상 소요시간 |
| 주변 편의시설 조회 | 정류장/역명, 카테고리 | 편의시설 이름, 종류, 주소 |
| 즐겨찾기 조회 | 없음 | 저장된 즐겨찾기 목록 |
| 즐겨찾기 추가 | 유형, ID 또는 이름 | 저장 성공/중복 안내 |
| 최근 검색 기록 | 없음 | 최근 검색 유형, 키워드, 시간 |

---

## 9. 비기능 요구사항

| ID | 항목 | 목표 |
|----|------|------|
| NFR-01 | 소켓 통신 | TCP 기반 서버/클라이언트 구조로 요청과 응답을 처리한다 |
| NFR-02 | 응답 시간 | 일반 검색은 로컬 테스트 환경에서 1초 이내 응답을 목표로 한다 |
| NFR-03 | 안정성 | 잘못된 입력, 빈 검색어, 클라이언트 비정상 종료 시 서버가 종료되지 않아야 한다 |
| NFR-04 | 파일 입출력 | 공공데이터 CSV, 로그, 즐겨찾기, 최근 검색 기록을 파일로 읽고 쓴다 |
| NFR-05 | 자료구조 요구 | 연결리스트를 직접 구현하고, 추가 자료구조로 그래프를 환승 기능에 적용한다 |
| NFR-06 | 이식성 | Windows와 Linux에서 빌드 가능한 C11 코드 구성을 목표로 한다 |
| NFR-07 | 데이터 품질 | CSV 컬럼 누락, 잘못된 참조, 중복 데이터에 대한 예외 처리를 포함한다 |
| NFR-08 | 로그 추적성 | 서버 로그에는 시간, 클라이언트 IP, 요청 타입, 처리 결과를 남긴다 |
| NFR-09 | 한글 처리 | CSV, 콘솔 출력, 프로토콜 문자열은 UTF-8 기준으로 처리한다 |
| NFR-10 | 범위 명확성 | 실시간 API 없이 정적 CSV 및 테스트 데이터 기반으로 기능을 구현한다 |

---

## 10. 예상 파일 구조

```text
public_transport_system/
├── src/
│   ├── server/
│   │   ├── main.c
│   │   ├── server.c / server.h
│   │   ├── client_handler.c / client_handler.h
│   │   ├── router.c / router.h
│   │   ├── search_service.c / search_service.h
│   │   ├── transfer_service.c / transfer_service.h
│   │   ├── data_loader.c / data_loader.h
│   │   └── log.c / log.h
│   │
│   ├── client/
│   │   ├── main.c
│   │   ├── menu.c / menu.h
│   │   ├── client_net.c / client_net.h
│   │   ├── favorite.c / favorite.h
│   │   └── recent_search.c / recent_search.h
│   │
│   └── common/
│       ├── protocol.h
│       ├── types.h
│       ├── linked_list.c / linked_list.h
│       ├── graph.c / graph.h
│       ├── csv_parser.c / csv_parser.h
│       ├── string_utils.c / string_utils.h
│       └── net_compat.h
│
├── data/
│   ├── bus_stops.csv
│   ├── bus_routes.csv
│   ├── route_stops.csv
│   ├── subway_stations.csv
│   ├── facilities.csv
│   ├── arrival_test.csv
│   └── crowding_test.csv
│
├── client_data/
│   ├── favorites.txt
│   └── recent_search.txt
│
├── logs/
│   └── server.log
│
├── Makefile
├── requirement.md
└── README.md
```

---

## 11. 개발 우선순위

| 우선순위 | FR 범위 | 핵심 이유 |
|----------|---------|-----------|
| P0 (필수) | FR-N01~N03, FR-DATA01~DATA03 | 서버/클라이언트 연결, CSV 로드, 자료구조 저장이 전체 기능의 기반 |
| P1 (핵심) | FR-S01~S03 | 정류장, 버스, 지하철역 검색은 최우선 사용자 기능 |
| P2 (확장) | FR-S04~S05, FR-DATA05 | 환승 시스템과 편의시설 추천으로 프로젝트 차별화 |
| P3 (편의) | FR-U01~U06, FR-N04 | 즐겨찾기, 최근 검색, 로그 저장으로 사용성 및 검증 가능성 확보 |
| P4 (표시) | FR-O01~O04 | 도착 예정시간, 혼잡도, 운행 상태는 테스트 데이터 가공 후 완성 |
| Out-of-Scope | FR-X01~X05 | 현재 프로젝트 범위 밖 기능 |

---

## 12. 요구사항 검증 체크리스트

| 검증 항목 | 관련 요구사항 | 충족 기준 |
|-----------|---------------|-----------|
| 채팅/학생정보관리 제외 | FR-X01, FR-X02 | 문서와 기능 목록에 해당 주제가 포함되지 않음 |
| 소켓 프로그래밍 | FR-N01~N03, NFR-01 | 서버와 클라이언트가 TCP로 요청/응답 수행 |
| 연결리스트 구현 | FR-DATA02, NFR-05 | 정류장/노선/편의시설 저장에 직접 구현한 연결리스트 사용 |
| 추가 자료구조 적용 | FR-DATA03, FR-S04, NFR-05 | 환승 정보 조회에 그래프 자료구조 사용 |
| 공공데이터 CSV 사용 | FR-DATA01, NFR-04 | 공공데이터포털 CSV를 로드하고 검색 기능에 활용 |
| 파일 입출력 | FR-DATA01, FR-U04, FR-N04 | CSV 읽기, 최근 검색 저장, 서버 로그 저장 동작 |
| 정류장 검색 | FR-S01 | 정류장별 경유 버스 조회 가능 |
| 버스 검색 | FR-S02 | 버스 번호별 경유 정류장 순서 조회 가능 |
| 지하철역 검색 | FR-S03 | 역명/키워드 검색 가능 |
| 즐겨찾기 | FR-U01~U03 | 추가, 조회, 중복 방지 가능 |
| 최근 검색 기록 | FR-U04~U05 | 최근 검색 내용이 txt 파일에 저장되고 메뉴에서 조회 가능 |
| 운행 상태 | FR-O01 | 운행 시작, 곧 도착, 운행 종료 표시 가능 |
| 도착 예정시간 | FR-O02, FR-DATA05 | 테스트 데이터 기반 도착 예정시간 표시 가능 |
| 환승 시스템 | FR-S04 | 출발지-도착지 간 추천 경로 출력 가능 |
| 편의시설 추천 | FR-S05 | CSV에 있는 편의시설을 주변 정보로 추천 가능 |
| 혼잡도 표시 | FR-O03~O04 | API 없이 시간대별 테스트 데이터 기반 표시 가능 |
| 최종 메뉴 구성 | FR-U06 | overview.md의 1~8, 0번 메뉴와 일치 |
