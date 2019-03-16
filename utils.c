#include "directed_graph_node.h"
#include "unordered_map.h"
#include "utils.h"
#include "list.h"
#include <math.h>
#include <stdio.h>

point_3d* random_point(double maxx, double maxy, double maxz)
{
    point_3d* p_ret = malloc(sizeof(*p_ret));

    if (!p_ret) return NULL;

    p_ret->x = ((double)rand() / (double)RAND_MAX) * maxx;
    p_ret->y = ((double)rand() / (double)RAND_MAX) * maxy;
    p_ret->z = ((double)rand() / (double)RAND_MAX) * maxz;

    return p_ret;
}

double point_3d_distance(point_3d* p_a, point_3d* p_b)
{
    double dx = p_a->x - p_b->x;
    double dy = p_a->y - p_b->y;
    double dz = p_a->z - p_b->z;

    return sqrt(dx * dx + dy * dy + dz * dz);
}

directed_graph_node* choose(directed_graph_node** p_table,
    const size_t size)
{
    size_t index = rand() % size;
    return p_table[index];
}

graph_data* create_random_graph(const size_t nodes,
    size_t edges,
    const double maxx,
    const double maxy,
    const double maxz)
{
    size_t i;
    char* p_name;

    directed_graph_node*            p_tail;
    directed_graph_node*            p_head;
    directed_graph_weight_function* p_weight_function;
    unordered_map*                  p_point_map;
    point_3d*                       p_a;
    point_3d*                       p_b;
    graph_data*                     p_ret;

    p_ret = malloc(sizeof(*p_ret));

    if (!p_ret) return NULL;

    directed_graph_node** p_node_array =
        malloc(sizeof(directed_graph_node*) * nodes);

    if (!p_ret)
    {
        free(p_ret);
        return NULL;
    }

    if (!(p_weight_function =
        directed_graph_weight_function_alloc(hash_function,
            equals_function)))
    {
        free(p_ret);
        free(p_node_array);
        return NULL;
    }

    if (!(p_point_map = unordered_map_alloc(16,
        1.0f,
        hash_function,
        equals_function)))
    {
        directed_graph_weight_function_free(p_weight_function);
        free(p_ret);
        free(p_node_array);
        return NULL;
    }

    for (i = 0; i < nodes; ++i)
    {
        p_name = malloc(sizeof(char) * 20);
 //       sprintf_s(p_name, "%d", i);
        p_node_array[i] = directed_graph_node_alloc(p_name);
        //        printf("%s\n", directed_graph_node_t_to_string(p_node_array[i]));
        unordered_map_put(p_point_map,
            p_node_array[i],
            random_point(maxx, maxy, maxz));
    }

    while (edges > 0)
    {
        p_tail = choose(p_node_array, nodes);
        p_head = choose(p_node_array, nodes);

        p_a = unordered_map_get(p_point_map, p_tail);
        p_b = unordered_map_get(p_point_map, p_head);

        directed_graph_node_add_arc(p_tail, p_head);

        directed_graph_weight_function_put(
            p_weight_function,
            p_tail,
            p_head,
            1.2 * point_3d_distance(p_a, p_b));

        --edges;
    }

    p_ret->p_node_array = p_node_array;
    p_ret->p_weight_function = p_weight_function;
    p_ret->p_point_map = p_point_map;

    return p_ret;
}

list* traceback_path(directed_graph_node* p_target,
    unordered_map* p_parent_map)
{
    list* p_ret;
    directed_graph_node* p_current;

    if (!p_target)     return NULL;
    if (!p_parent_map) return NULL;

    p_ret = list_alloc(10);

    if (!p_ret)        return NULL;

    p_current = p_target;

    while (p_current)
    {
        list_push_front(p_ret, p_current);
        p_current = unordered_map_get(p_parent_map, p_current);
    }

    return p_ret;
}

bool is_valid_path(list* p_path)
{
    size_t i;
    size_t sz;

    if (!p_path) return false;

    /* A empty path is defined to be valid. */
    if ((sz = list_size(p_path)) == 0) return true;

    for (i = 0; i < sz - 1; ++i)
    {
        if (!directed_graph_node_has_child(list_get(p_path, i),
            list_get(p_path, i + 1)))
        {
            return false;
        }
    }

    return true;
}

double compute_path_cost(list* p_path,
    directed_graph_weight_function* p_weight_function)
{
    size_t i;
    size_t sz;
    double cost = 0.0;

    if (!p_path) return 0.0;

    /* A empty path is defined to be valid. */
    if ((sz = list_size(p_path)) == 0) return 0.0;

    for (i = 0; i < sz - 1; ++i)
    {
        cost += *directed_graph_weight_function_get(p_weight_function,
            list_get(p_path, i),
            list_get(p_path, i + 1));
    }

    return cost;
}