#include "dijkstra.h"
#include "list.h"
#include "directed_graph_node.h"
#include "weight_function.h"
#include "unordered_map.h"
#include "unordered_set.h"
#include "heap.h"
#include "utils.h"

typedef struct weight {
    double weight;
} weight;

static int priority_cmp(void* pa, void* pb)
{
    double da;
    double db;

    da = ((weight*)pa)->weight;
    db = ((weight*)pb)->weight;

    if (da < db)
    {
        return -1;
    }
    else if (da > db)
    {
        return 1;
    }

    return 0;
}

static const size_t INITIAL_CAPACITY = 16;
static const float  LOAD_FACTOR = 1.0f;

list* dijkstra(directed_graph_node* p_source,
               directed_graph_node* p_target,
               directed_graph_weight_function* p_weight_function)
{
    list*                   p_list;
    heap*                   p_open_set;
    unordered_set*          p_closed_set;
    unordered_map*          p_parent_map;
    unordered_map*          p_cost_map;
    directed_graph_node*    p_current;
    directed_graph_node*    p_child;
    unordered_set_iterator* p_child_iterator;
    weight*                 p_weight;
    list*                   p_weight_list;
    size_t                  i;

    if (!p_source)          return NULL;
    if (!p_target)          return NULL;
    if (!p_weight_function) return NULL;

    p_open_set = heap_alloc(4,
                            INITIAL_CAPACITY,
                            LOAD_FACTOR,
                            hash_function,
                            equals_function,
                            priority_cmp);

    if (!p_open_set)
    {
        return NULL;
    }

    p_closed_set = unordered_set_alloc(INITIAL_CAPACITY,
                                       LOAD_FACTOR,
                                       hash_function,
                                       equals_function);

    if (!p_closed_set)
    {
        heap_free(p_open_set);
        return NULL;
    }

    p_parent_map = unordered_map_alloc(INITIAL_CAPACITY,
                                       LOAD_FACTOR,
                                       hash_function,
                                       equals_function);

    if (!p_parent_map)
    {
        heap_free(p_open_set);
        unordered_set_free(p_closed_set);
        return NULL;
    }

    p_cost_map = unordered_map_alloc(INITIAL_CAPACITY,
        LOAD_FACTOR,
        hash_function,
        equals_function);

    if (!p_cost_map)
    {
        heap_free(p_open_set);
        unordered_set_free(p_closed_set);
        unordered_map_free(p_parent_map);
        return NULL;
    }

    p_weight_list = list_alloc(INITIAL_CAPACITY);

    if (!p_weight_list)
    {
        heap_free(p_open_set);
        unordered_set_free(p_closed_set);
        unordered_map_free(p_parent_map);
        unordered_map_free(p_cost_map);
        return NULL;
    }

    p_weight = malloc(sizeof(*p_weight));
    p_weight->weight = 0.0;

    heap_add(p_open_set, p_source, p_weight);
    unordered_map_put(p_parent_map, p_source, NULL);
    unordered_map_put(p_cost_map, p_source, p_weight);
    list_push_back(p_weight_list, p_weight);

    while (heap_size(p_open_set) > 0)
    {
        p_current = heap_extract_min(p_open_set);

        if (equals_function(p_current, p_target))
        {
            p_list = traceback_path(p_target, p_parent_map);
            heap_free(p_open_set);
            unordered_set_free(p_closed_set);
            unordered_map_free(p_parent_map);
            unordered_map_free(p_cost_map);
            return p_list;
        }

        unordered_set_add(p_closed_set, p_current);

        p_child_iterator =
            unordered_set_iterator_alloc(
                directed_graph_node_children_set(p_current));

        while (unordered_set_iterator_has_next(p_child_iterator))
        {
            unordered_set_iterator_next(p_child_iterator, &p_child);

            if (unordered_set_contains(p_closed_set, p_child)) {
                continue;
            }

            double tmp_cost =
                ((weight*)unordered_map_get(p_cost_map,
                    p_current))->weight;

            tmp_cost += *directed_graph_weight_function_get(
                p_weight_function,
                p_current,
                p_child);

            if (!unordered_map_contains_key(p_parent_map, p_child))
            {
                p_weight = malloc(sizeof(*p_weight));
                p_weight->weight = tmp_cost;

                heap_add(p_open_set, p_child, p_weight);
                unordered_map_put(p_parent_map, p_child, p_current);
                unordered_map_put(p_cost_map, p_child, p_weight);
            }
            else if (tmp_cost <
                ((weight*)unordered_map_get(p_cost_map,
                    p_child))->weight)
            {
                p_weight = malloc(sizeof(*p_weight));
                p_weight->weight = tmp_cost;

                heap_decrease_key(p_open_set, p_child, p_weight);
                unordered_map_put(p_parent_map, p_child, p_current);
                unordered_map_put(p_cost_map, p_child, p_weight);
            }
        }

        unordered_set_iterator_free(p_child_iterator);
    }

    /* Once here, return a empty path in order to denote the fact that the
       target node is not reachable from source node. */
    heap_free(p_open_set);
    unordered_set_free(p_closed_set);
    unordered_map_free(p_parent_map);
    unordered_map_free(p_cost_map);
    p_list = list_alloc(10);

    /* Deallocate the weights. */
    for (i = 0; i < list_size(p_weight_list); ++i)
    {
        free(list_get(p_weight_list, i));
    }

    return p_list;
}