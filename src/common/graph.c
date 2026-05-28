#include "graph.h"

#include "string_utils.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

// 그래프를 빈 상태로 준비합니다.
void graph_init(TransferGraph *graph) {
    // 그래프 주소가 전달된 경우에만 초기화 값을 설정합니다.
    if (!graph) {
        return;
    }
    // 노드 배열과 개수를 초기값으로 맞춥니다.
    graph->nodes = NULL;
    graph->node_count = 0;
    graph->capacity = 0;
}

// 그래프에 잡아 둔 메모리를 모두 해제합니다.
void graph_free(TransferGraph *graph) {
    int i;
    // 해제 대상 그래프가 전달되지 않으면 정리 작업을 생략합니다.
    if (!graph) {
        return;
    }
    // 각 노드에 연결된 간선 목록을 먼저 지웁니다.
    for (i = 0; i < graph->node_count; ++i) {
        GraphEdge *edge = graph->nodes[i].edges;
        while (edge) {
            GraphEdge *next = edge->next;
            free(edge);
            edge = next;
        }
    }
    // 노드 배열을 해제한 뒤 그래프 상태를 초기값으로 되돌립니다.
    free(graph->nodes);
    graph_init(graph);
}

// 필요한 만큼 노드 배열 크기를 늘립니다.
static int graph_reserve(TransferGraph *graph, int needed) {
    GraphNode *new_nodes;
    int new_capacity;
    // 이미 공간이 충분하면 그대로 사용합니다.
    if (graph->capacity >= needed) {
        return 1;
    }
    // 처음에는 16칸, 이후에는 2배씩 늘립니다.
    new_capacity = graph->capacity == 0 ? 16 : graph->capacity * 2;
    while (new_capacity < needed) {
        new_capacity *= 2;
    }
    // 기존 배열을 새 크기로 다시 잡습니다.
    new_nodes = (GraphNode *)realloc(graph->nodes, (size_t)new_capacity * sizeof(*new_nodes));
    if (!new_nodes) {
        return 0;
    }
    // 새로 늘어난 부분은 0으로 채웁니다.
    memset(new_nodes + graph->capacity, 0, (size_t)(new_capacity - graph->capacity) * sizeof(*new_nodes));
    graph->nodes = new_nodes;
    graph->capacity = new_capacity;
    return 1;
}

// 노드 ID로 그래프 안의 위치를 찾습니다.
int graph_find_node(const TransferGraph *graph, const char *node_id) {
    int i;
    // 그래프 또는 노드 ID가 유효하지 않으면 검색 실패 값(-1)을 반환합니다.
    if (!graph || !node_id) {
        return -1;
    }
    // 앞에서부터 노드 ID를 비교합니다.
    for (i = 0; i < graph->node_count; ++i) {
        if (strcmp(graph->nodes[i].node_id, node_id) == 0) {
            return i;
        }
    }
    return -1;
}

// 그래프에 새 노드를 추가합니다.
int graph_add_node(TransferGraph *graph, const char *node_id, const char *node_name, const char *node_type, double lat, double lng) {
    int existing;
    GraphNode *node;
    // 노드 생성에 필요한 ID, 이름, 종류가 모두 있어야 추가를 진행합니다.
    if (!graph || !node_id || !node_name || !node_type) {
        return -1;
    }
    // 이미 있는 노드면 기존 위치를 그대로 돌려줍니다.
    existing = graph_find_node(graph, node_id);
    if (existing >= 0) {
        return existing;
    }
    // 새 노드가 들어갈 공간을 확보합니다.
    if (!graph_reserve(graph, graph->node_count + 1)) {
        return -1;
    }
    // 노드 정보를 배열 마지막 칸에 저장합니다.
    node = &graph->nodes[graph->node_count];
    safe_strcpy(node->node_id, sizeof(node->node_id), node_id);
    safe_strcpy(node->node_name, sizeof(node->node_name), node_name);
    safe_strcpy(node->node_type, sizeof(node->node_type), node_type);
    node->lat = lat;
    node->lng = lng;
    node->edges = NULL;
    graph->node_count++;
    return graph->node_count - 1;
}

// 특정 노드에서 목적지로 가는 간선을 찾습니다.
static GraphEdge *graph_get_edge(GraphNode *node, int to_index) {
    GraphEdge *edge;
    // 연결 리스트를 따라가며 목적지 번호를 비교합니다.
    for (edge = node->edges; edge; edge = edge->next) {
        if (edge->to_index == to_index) {
            return edge;
        }
    }
    return NULL;
}

// 경로를 만들 때 쓸 간선을 찾습니다.
static const GraphEdge *graph_find_edge(const TransferGraph *graph, int from_index, int to_index) {
    const GraphEdge *edge;
    // 출발 노드 번호가 잘못되면 찾지 않습니다.
    if (!graph || from_index < 0 || from_index >= graph->node_count) {
        return NULL;
    }
    // 출발 노드의 간선 목록에서 도착 노드를 찾습니다.
    for (edge = graph->nodes[from_index].edges; edge; edge = edge->next) {
        if (edge->to_index == to_index) {
            return edge;
        }
    }
    return NULL;
}

// 두 노드 사이에 이동 간선을 추가합니다.
int graph_add_edge(TransferGraph *graph, int from_index, int to_index, int weight_min, const char *route_no) {
    GraphEdge *edge;
    GraphEdge *existing;
    // 노드 인덱스가 그래프 범위를 벗어나면 간선 추가를 중단합니다.
    if (!graph || from_index < 0 || to_index < 0 ||
        from_index >= graph->node_count || to_index >= graph->node_count) {
        return 0;
    }
    // 자기 자신으로 가는 간선은 따로 만들지 않습니다.
    if (from_index == to_index) {
        return 1;
    }
    // 이미 간선이 있으면 더 짧은 시간만 반영합니다.
    existing = graph_get_edge(&graph->nodes[from_index], to_index);
    if (existing) {
        if (weight_min > 0 && weight_min < existing->weight_min) {
            existing->weight_min = weight_min;
            safe_strcpy(existing->route_no, sizeof(existing->route_no), route_no ? route_no : "");
        }
        return 1;
    }
    // 연결 정보를 담을 간선 노드를 동적으로 할당합니다.
    edge = (GraphEdge *)calloc(1, sizeof(*edge));
    if (!edge) {
        return 0;
    }
    // 도착 노드, 시간, 노선 번호를 저장합니다.
    edge->to_index = to_index;
    edge->weight_min = weight_min > 0 ? weight_min : 1;
    safe_strcpy(edge->route_no, sizeof(edge->route_no), route_no ? route_no : "");
    // 출발 노드의 간선 목록 앞에 붙입니다.
    edge->next = graph->nodes[from_index].edges;
    graph->nodes[from_index].edges = edge;
    return 1;
}

// 출발 노드에서 도착 노드까지 가장 짧은 경로를 찾습니다.
int graph_find_path(const TransferGraph *graph, int from_index, int to_index, GraphPath *path) {
    int *prev;
    int *dist;
    int *visited;
    int found = 0;
    int cur;
    int i;
    const int inf = INT_MAX / 4;

    // 출발지와 도착지 인덱스가 유효한 경우에만 최단 경로 탐색을 시작합니다.
    if (!graph || !path || from_index < 0 || to_index < 0 ||
        from_index >= graph->node_count || to_index >= graph->node_count) {
        return 0;
    }

    // 결과 경로를 먼저 비웁니다.
    memset(path, 0, sizeof(*path));
    // 최단 경로 탐색에 필요한 이전 노드, 거리, 방문 여부 배열을 할당합니다.
    prev = (int *)calloc((size_t)graph->node_count, sizeof(int));
    dist = (int *)calloc((size_t)graph->node_count, sizeof(int));
    visited = (int *)calloc((size_t)graph->node_count, sizeof(int));
    // 탐색용 배열을 확보하지 못하면 이미 할당된 메모리를 정리하고 실패를 반환합니다.
    if (!prev || !dist || !visited) {
        free(prev);
        free(dist);
        free(visited);
        return 0;
    }

    // 모든 노드의 거리를 아주 큰 값으로 시작합니다.
    for (i = 0; i < graph->node_count; ++i) {
        prev[i] = -1;
        dist[i] = inf;
    }

    // 출발 노드의 거리는 0으로 둡니다.
    dist[from_index] = 0;
    prev[from_index] = from_index;

    // 아직 방문하지 않은 노드 중 거리가 가장 짧은 노드를 계속 고릅니다.
    for (;;) {
        int best_dist = inf;
        const GraphEdge *edge;
        cur = -1;
        // 이번에 처리할 노드를 찾습니다.
        for (i = 0; i < graph->node_count; ++i) {
            if (!visited[i] && dist[i] < best_dist) {
                best_dist = dist[i];
                cur = i;
            }
        }
        // 방문 가능한 후보 노드가 남아 있지 않으면 탐색 반복을 종료합니다.
        if (cur < 0) {
            break;
        }
        // 가장 가까운 후보 노드를 확정하고 방문 표시를 남깁니다.
        visited[cur] = 1;
        // 도착 노드에 왔으면 경로를 찾은 것입니다.
        if (cur == to_index) {
            found = 1;
            break;
        }
        // 현재 노드와 연결된 노드들의 거리를 갱신합니다.
        for (edge = graph->nodes[cur].edges; edge; edge = edge->next) {
            int next = edge->to_index;
            int next_dist = dist[cur] + edge->weight_min;
            if (!visited[next] && next_dist < dist[next]) {
                dist[next] = next_dist;
                prev[edge->to_index] = cur;
            }
        }
    }

    // 도착 노드가 확정되면 이전 노드 배열을 역추적해 실제 경로를 복원합니다.
    if (found) {
        int rev[MAX_GRAPH_PATH];
        int count = 0;
        cur = to_index;
        // 도착지에서 출발지 방향으로 거꾸로 저장합니다.
        while (count < MAX_GRAPH_PATH) {
            rev[count++] = cur;
            if (cur == from_index) {
                break;
            }
            cur = prev[cur];
        }
        // 출발지까지 제대로 이어졌는지 확인합니다.
        if (count > 0 && rev[count - 1] == from_index) {
            path->count = count;
            path->total_minutes = dist[to_index];
            // 거꾸로 저장한 경로를 다시 정방향으로 옮깁니다.
            for (i = 0; i < count; ++i) {
                path->indices[i] = rev[count - i - 1];
            }
            // 각 구간의 이동 시간과 노선 번호를 저장합니다.
            for (i = 0; i + 1 < count; ++i) {
                const GraphEdge *edge = graph_find_edge(graph, path->indices[i], path->indices[i + 1]);
                if (edge) {
                    path->segment_minutes[i] = edge->weight_min;
                    safe_strcpy(path->segment_routes[i], sizeof(path->segment_routes[i]), edge->route_no);
                }
            }
        } else {
            found = 0;
        }
    }

    // 임시 배열을 모두 해제합니다.
    free(prev);
    free(dist);
    free(visited);
    return found && path->count > 0;
}
