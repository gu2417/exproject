# 저장소 구조

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
│   │   ├── facility_service.c / facility_service.h
│   │   ├── operation_service.c / operation_service.h
│   │   ├── data_loader.c / data_loader.h
│   │   └── log.c / log.h
│   ├── client/
│   │   ├── main.c
│   │   ├── menu.c / menu.h
│   │   ├── client_net.c / client_net.h
│   │   ├── request_builder.c / request_builder.h
│   │   ├── result_viewer.c / result_viewer.h
│   │   ├── favorite.c / favorite.h
│   │   └── recent_search.c / recent_search.h
│   └── common/
│       ├── protocol.h
│       ├── types.h
│       ├── linked_list.c / linked_list.h
│       ├── graph.c / graph.h
│       ├── csv_parser.c / csv_parser.h
│       ├── string_utils.c / string_utils.h
│       └── net_compat.h
├── data/
├── client_data/
├── logs/
├── docs/
├── Makefile
├── requirement.md
└── README.md
```

## 원칙

- 서버, 클라이언트, 공통 모듈을 분리한다.
- 패킷 타입과 공통 구조체는 `src/common/`에서 공유한다.
- CSV 로딩과 검색 로직은 서버 쪽에 둔다.
- 즐겨찾기와 최근 검색은 클라이언트 로컬 기능으로 둔다.

