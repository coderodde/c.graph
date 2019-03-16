#include "weight_function.h"
#include "unordered_map.h"

typedef struct directed_graph_weight_function_state {
    unordered_map* p_first_level_map;
    size_t(*p_hash_function)(void*);
    bool(*p_equals_function)(void*, void*);
} directed_graph_weight_function_state;

static size_t INITIAL_CAPACITY = 16;
static size_t LOAD_FACTOR = 1.0f;

directed_graph_weight_function* directed_graph_weight_function_alloc
(size_t(*p_hash_function)(void*),
    bool(*p_equals_function)(void*, void*))
{
    directed_graph_weight_function* p_ret;

    if (!p_hash_function)   return NULL;
    if (!p_equals_function) return NULL;

    p_ret = malloc(sizeof(*p_ret));

    if (!p_ret) return NULL;

    p_ret->state = malloc(sizeof(*p_ret->state));

    p_ret->state->p_first_level_map = unordered_map_alloc(INITIAL_CAPACITY,
        LOAD_FACTOR,
        p_hash_function,
        p_equals_function);
    p_ret->state->p_hash_function = p_hash_function;
    p_ret->state->p_equals_function = p_equals_function;
    return p_ret;
}

bool directed_graph_weight_function_put
(directed_graph_weight_function* p_weight_function,
    directed_graph_node* p_tail,
    directed_graph_node* p_head,
    double weight)
{
    unordered_map* p_tmp_map;
    double* p_weight;

    if (!p_weight_function) return false;
    if (!p_tail)            return false;
    if (!p_head)            return false;

    p_tmp_map = unordered_map_get(p_weight_function->state->p_first_level_map,
        p_tail);

    if (p_tmp_map)
    {
        p_weight = malloc(sizeof(double));
        *p_weight = weight;
        unordered_map_put(p_tmp_map, p_head, p_weight);
        return unordered_map_contains_key(p_tmp_map, p_head);
    }

    p_tmp_map = unordered_map_alloc(INITIAL_CAPACITY,
        LOAD_FACTOR,
        p_weight_function->state->p_hash_function,
        p_weight_function->state->p_equals_function);

    if (!p_tmp_map) return false;

    unordered_map_put(p_weight_function->state->p_first_level_map,
        p_tail,
        p_tmp_map);

    if (!unordered_map_contains_key(p_weight_function->state->p_first_level_map,
        p_tail)) return false;

    p_weight = malloc(sizeof(double));
    *p_weight = weight;

    unordered_map_put(p_tmp_map, p_head, p_weight);

    if (!unordered_map_contains_key(p_tmp_map, p_head))
    {
        free(p_weight);
        return false;
    }

    return true;
}

double* directed_graph_weight_function_get(
    directed_graph_weight_function* p_function,
    directed_graph_node* p_tail,
    directed_graph_node* p_head)
{
    unordered_map* p_second_level_map;

    if (!p_function) return NULL;
    if (!p_tail)     return NULL;
    if (!p_head)     return NULL;

    if (!(p_second_level_map = unordered_map_get(
        p_function->state->p_first_level_map, p_tail)))
    {
        return NULL;
    }

    return unordered_map_get(p_second_level_map, p_head);
}

void directed_graph_weight_function_free
(directed_graph_weight_function* p_function)
{
    unordered_map_iterator* p_iterator;
    unordered_map_iterator* p_iterator_2;
    unordered_map*          p_map;
    directed_graph_node*    p_node;
    directed_graph_node*    p_node_2;
    double*                   p_weight;

    if (!p_function) return;

    p_iterator = unordered_map_iterator_alloc(p_function->state->p_first_level_map);

    while (unordered_map_iterator_has_next(p_iterator))
    {
        unordered_map_iterator_next(p_iterator, &p_node, &p_map);
        p_iterator_2 = unordered_map_iterator_alloc(p_map);

        while (unordered_map_iterator_has_next(p_iterator_2))
        {
            unordered_map_iterator_next(p_iterator_2, &p_node_2, &p_weight);
            free(p_weight);
        }

        unordered_map_free(p_map);
    }

    unordered_map_free(p_function->state->p_first_level_map);
}
