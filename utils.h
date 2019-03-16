#ifndef GRAPH_UTILS_H
#define GRAPH_UTILS_H

#include "directed_graph_node.h"
#include "unordered_map.h"
#include "weight_function.h"
#include "list.h"

#ifdef  __cplusplus
extern "C" {
#endif

    typedef struct point_3d {
        double x;
        double y;
        double z;
    } point_3d;

    typedef struct graph_data {
        directed_graph_node**           p_node_array;
        directed_graph_weight_function* p_weight_function;
        unordered_map*                  p_point_map;
    } graph_data;

    point_3d* random_point(double maxx, double maxy, double maxz);

    double point_3d_distance(point_3d* p_a, point_3d* p_b);

    directed_graph_node* choose(directed_graph_node** p_table,
        const size_t size);

    graph_data* create_random_graph(const size_t nodes,
        size_t edges,
        const double maxx,
        const double maxy,
        const double maxz);

    list* traceback_path(directed_graph_node* p_target,
        unordered_map* p_parent_map);

    bool is_valid_path(list* p_path);

    double compute_path_cost(
        list* p_path, directed_graph_weight_function* p_weight_function);

#ifdef  __cplusplus
}
#endif

#endif  /* GRAPH_UTILS_H */