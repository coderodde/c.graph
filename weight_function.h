#ifndef WEIGHT_FUNCTION_H
#define WEIGHT_FUNCTION_H

#include "directed_graph_node.h"
#include <stdbool.h>

#ifdef  __cplusplus
extern "C" {
#endif

    typedef struct directed_graph_weight_function {
        struct directed_graph_weight_function_state* state;
    } directed_graph_weight_function;

    /***************************************************************************
    * Allocates a new, empty weight function.                                  *
    ***************************************************************************/
    directed_graph_weight_function*
        directed_graph_weight_function_alloc(size_t(*p_hash_function)(void*),
                                             bool(*p_equals_function)(void*, void*));

    /***************************************************************************
    * Associates the weight 'weight' with the arc ('p_tail', 'p_head').        *
    ***************************************************************************/
    bool directed_graph_weight_function_put(
        directed_graph_weight_function* p_function,
        directed_graph_node* p_tail,
        directed_graph_node* p_head,
        double weight);

    /***************************************************************************
    * Reads the weight for the arc ('p_tail', 'p_head').                       *
    ***************************************************************************/
    double* directed_graph_weight_function_get(
        directed_graph_weight_function* p_function,
        directed_graph_node* p_tail,
        directed_graph_node* p_head);

    /***************************************************************************
    * Deallocate the weight function.                                          *
    ***************************************************************************/
    void directed_graph_weight_function_free
    (directed_graph_weight_function* p_function);

#ifdef  __cplusplus
}
#endif

#endif  /* WEIGHT_FUNCTION_H */