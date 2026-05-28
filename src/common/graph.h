#ifndef TRANSIT_GRAPH_H
#define TRANSIT_GRAPH_H

#include "types.h"

// 환승 그래프를 준비하고 해제합니다.
void graph_init(TransferGraph *graph);
void graph_free(TransferGraph *graph);
// 그래프에 노드와 간선을 넣거나 노드를 찾습니다.
int graph_add_node(TransferGraph *graph, const char *node_id, const char *node_name, const char *node_type, double lat, double lng);
int graph_find_node(const TransferGraph *graph, const char *node_id);
int graph_add_edge(TransferGraph *graph, int from_index, int to_index, int weight_min, const char *route_no);
// 출발 노드에서 도착 노드까지 경로를 찾습니다.
int graph_find_path(const TransferGraph *graph, int from_index, int to_index, GraphPath *path);

#endif
