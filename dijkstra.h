#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include "directed_graph_node.h"
#include "weight_function.h"
#include "list.h"

#ifdef  __cplusplus
extern "C" {
#endif

    list* dijkstra(directed_graph_node* p_source,
                   directed_graph_node* p_target,
                   directed_graph_weight_function* p_weight_function);

#ifdef  __cplusplus
}
#endif

#endif  /* DIJKSTRA_H */