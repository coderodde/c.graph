#include "directed_graph_node.h"
#include "unordered_set.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

typedef struct directed_graph_node_state {
    char* p_name;
    char* p_text;
    struct unordered_set* p_parent_node_set;
    struct unordered_set* p_child_node_set;
} directed_graph_node_state;

static const int INITIAL_CAPACITY = 16;
static const int MAXIMUM_NAME_STRING_LEN = 80;
static const float LOAD_FACTOR = 1.0f;

bool equals_function(void* a, void* b)
{
    if (!a || !b) return false;

    return strcmp(((directed_graph_node*)a)->state->p_name,
                  ((directed_graph_node*)b)->state->p_name) == 0;
}

/* TODO: Come up with a better hash function. */
size_t hash_function(void* v)
{
    size_t ret;
    size_t i;
    char* pc;

    if (!v) return 0;

    ret = 0;
    i = 1;
    pc = ((directed_graph_node*)v)->state->p_name;

    while (*pc)
    {
        ret += *pc * i;
        ++i;
        ++pc;
    }

    return ret;
}

static const size_t MAXIMUM_NAME_LENGTH = 80;

directed_graph_node* directed_graph_node_alloc(char* name)
{
    directed_graph_node* p_node = malloc(sizeof(*p_node));
    char* p_text; 
    
    p_node->state = malloc(sizeof(*p_node->state));

    if (!p_node) return NULL;

    p_node->state->p_name = name;
    p_text = malloc(sizeof(char) * MAXIMUM_NAME_STRING_LEN);

    if (!p_text)
    {
        free(p_node->state);
        free(p_node);
        return NULL;
    }

    p_node->state->p_child_node_set = unordered_set_alloc(INITIAL_CAPACITY,
                                                          LOAD_FACTOR,
                                                          hash_function,
                                                          equals_function);

    if (!p_node->state->p_child_node_set)
    {
        free(p_text);
        free(p_node->state);
        free(p_node);
        return NULL;
    }

    p_node->state->p_parent_node_set = 
        unordered_set_alloc(INITIAL_CAPACITY,
                            LOAD_FACTOR,
                            hash_function,
                            equals_function);

    if (!p_node->state->p_parent_node_set)
    {
        unordered_set_free(p_node->state->p_child_node_set);
        free(p_text);
        free(p_node);
        return NULL;
    }

    snprintf(p_text,
        MAXIMUM_NAME_STRING_LEN,
        "[directed_graph_node_t: id = %s]",
        name);

    p_node->state->p_name = name;
    p_node->state->p_text = p_text;
    return p_node;
}

bool
directed_graph_node_add_arc(directed_graph_node* p_tail,
                            directed_graph_node* p_head)
{
    if (!p_tail || !p_head) return false;

    if (!unordered_set_add(p_tail->state->p_child_node_set, p_head))
    {
        return false;
    }

    if (!unordered_set_add(p_head->state->p_parent_node_set, p_tail))
    {
        unordered_set_remove(p_tail->state->p_child_node_set, p_head);
        return false;
    }

    return true;
}

bool directed_graph_node_has_child(directed_graph_node* p_node, 
                                   directed_graph_node* p_child_candidate)
{
    if (!p_node || !p_child_candidate) return false;

    return unordered_set_contains(p_node->state->p_child_node_set,
        p_child_candidate);
}

bool directed_graph_node_remove_arc(directed_graph_node* p_tail,
    directed_graph_node* p_head)
{
    if (!p_tail || !p_head) return false;

    unordered_set_remove(p_tail->state->p_child_node_set, p_head);
    unordered_set_remove(p_head->state->p_parent_node_set, p_tail);
    return true;
}

char* directed_graph_node_to_string(directed_graph_node* p_node)
{
    if (!p_node) return "NULL node";

    return p_node->state->p_text;
}

struct unordered_set*
directed_graph_node_children_set(directed_graph_node* p_node)
{
    return p_node ? p_node->state->p_child_node_set : NULL;
}

struct unordered_set*
directed_graph_node_parent_set(directed_graph_node* p_node)
{
    return p_node ? p_node->state->p_parent_node_set : NULL;
}

void directed_graph_node_clear(directed_graph_node* p_node)
{
    unordered_set_iterator* p_iterator;
    directed_graph_node*    p_tmp_node;

    if (!p_node) return;

    p_iterator = unordered_set_iterator_alloc(p_node->state->p_child_node_set);

    while (unordered_set_iterator_has_next(p_iterator))
    {
        unordered_set_iterator_next(p_iterator, &p_tmp_node);

        if (strcmp(p_node->state->p_name, p_tmp_node->state->p_name) != 0)
        {
            unordered_set_remove(p_tmp_node->state->p_parent_node_set, p_node);
        }
    }

    p_iterator = unordered_set_iterator_alloc(p_node->state->p_parent_node_set);

    while (unordered_set_iterator_has_next(p_iterator))
    {
        unordered_set_iterator_next(p_iterator, &p_tmp_node);

        if (strcmp(p_node->state->p_name, p_tmp_node->state->p_name) != 0)
        {
            unordered_set_remove(p_tmp_node->state->p_child_node_set, p_node);
        }
    }

    unordered_set_clear(p_node->state->p_parent_node_set);
    unordered_set_clear(p_node->state->p_child_node_set);
}

void directed_graph_node_free(directed_graph_node* p_node)
{
    unordered_set_iterator* p_iterator;
    directed_graph_node* p_tmp_node;

    if (!p_node) return;

    directed_graph_node_clear(p_node);
    unordered_set_free(p_node->state->p_child_node_set);
    unordered_set_free(p_node->state->p_parent_node_set);
    free(p_node->state);
    free(p_node);
}
