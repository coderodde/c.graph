#include "heap.h"
#include "unordered_map.h"
#include <stdbool.h>

typedef struct heap_node {
    void*  element;
    void*  priority;
    size_t index;
} heap_node;

typedef struct heap_state {
    unordered_map* node_map;
    heap_node**    table;
    size_t(*hash_function)(void*);
    bool(*equals_function)(void*, void*);
    int(*key_compare_function)(void*, void*);
    size_t         size;
    size_t         capacity;
    size_t         degree;
    size_t*        indices;
} heap_state;
/*
typedef struct heap {
    struct heap_state* state;
} heap;*/

static heap_node* heap_node_alloc(void* element, void* priority)
{
    heap_node* p_ret = malloc(sizeof(*p_ret));

    if (!p_ret)
    {
        return NULL;
    }

    p_ret->element = element;
    p_ret->priority = priority;

    return p_ret;
}

static const size_t MINIMUM_CAPACITY = 16;

static size_t fix_degree(size_t degree)
{
    return degree < 2 ? 2 : degree;
}

static size_t fix_initial_capacity(size_t initial_capacity)
{
    return initial_capacity < MINIMUM_CAPACITY ? MINIMUM_CAPACITY :
        initial_capacity;
}

heap* heap_alloc(size_t   degree,
    size_t   initial_capacity,
    float    load_factor,
    size_t(*hash_function)(void*),
    bool(*equals_function)(void*, void*),
    int(*priority_compare_function)(void*, void*))
{
    heap* my_heap;
    unordered_map* p_map;

    if (!hash_function || !equals_function || !priority_compare_function)
    {
        return NULL;
    }

    my_heap = malloc(sizeof(*my_heap));

    if (!my_heap)
    {
        return NULL;
    }

    my_heap->state = malloc(sizeof(*my_heap->state));

    if (!my_heap)
    {
        free(my_heap);
        return NULL;
    }

    p_map = unordered_map_alloc(initial_capacity,
        load_factor,
        hash_function,
        equals_function);

    if (!p_map)
    {
        free(my_heap);
        return NULL;
    }

    degree = fix_degree(degree);
    initial_capacity = fix_initial_capacity(initial_capacity);

    my_heap->state->table = malloc(sizeof(heap_node*) * initial_capacity);

    if (!my_heap->state->table)
    {
        unordered_map_free(p_map);
        free(my_heap);
        return NULL;
    }

    my_heap->state->indices = malloc(sizeof(size_t) * degree);

    if (!my_heap->state->indices)
    {
        unordered_map_free(p_map);
        free(my_heap->state->table);
        free(my_heap);
        return NULL;
    }

    my_heap->state->node_map             = p_map;
    my_heap->state->capacity             = initial_capacity;
    my_heap->state->size                 = 0;
    my_heap->state->degree               = degree;
    my_heap->state->hash_function        = hash_function;
    my_heap->state->equals_function      = equals_function;
    my_heap->state->key_compare_function = priority_compare_function;

    return my_heap;
}

static size_t get_parent_index(heap* my_heap, size_t child_index)
{
    return (child_index - 1) / my_heap->state->degree;
}

/*******************************************************************************
* Sifts up the node until the minimum heap property is restored.               *
*******************************************************************************/
static void sift_up(heap* my_heap, size_t index)
{
    size_t parent_index;
    heap_node* p_target_node;
    heap_node* p_parent_node;

    if (index == 0)
    {
        return;
    }

    parent_index = get_parent_index(my_heap, index);
    p_target_node = my_heap->state->table[index];

    while (true)
    {
        p_parent_node = my_heap->state->table[parent_index];

        if (my_heap->state->key_compare_function(p_parent_node->priority,
            p_target_node->priority) > 0)
        {
            my_heap->state->table[index] = p_parent_node;
            p_parent_node->index = index;
            index = parent_index;
            parent_index = get_parent_index(my_heap, index);
        }
        else
        {
            break;
        }

        if (index == 0)
        {
            break;
        }
    }

    my_heap->state->table[index] = p_target_node;
    p_target_node->index = index;
}

/*******************************************************************************
* Loads the indices of child nodes relative to the node with index 'index'.    *
*******************************************************************************/
static void compute_children_indices(heap* my_heap, size_t index)
{
    size_t degree = my_heap->state->degree;
    size_t i;

    for (i = 0; i < degree; ++i)
    {
        my_heap->state->indices[i] = degree * index + i + 1;

        if (my_heap->state->indices[i] >= my_heap->state->size)
        {
            my_heap->state->indices[i] = (size_t)-1;
            return;
        }
    }
}

bool heap_is_healthy(heap* my_heap)
{
    size_t i;
    size_t j;
    size_t child_index;

    if (!my_heap)
    {
        return false;
    }

    for (i = 0; i < my_heap->state->size; ++i)
    {
        /* Check that all the children of the current node has priorities no
           less than the node itself. */
        compute_children_indices(my_heap, i);

        for (j = 0; j < my_heap->state->degree; ++j)
        {
            child_index = my_heap->state->indices[j];

            if (child_index != (size_t)-1)
            {
                if (my_heap->state->
                    key_compare_function(
                        my_heap->state->table[i]->priority,
                        my_heap->state->table[child_index]->priority) > 0)
                {
                    return false;
                }
            }
            else
            {
                break;
            }
        }
    }

    return true;
}

/*******************************************************************************
* Sifts down the node at the root to location that maintains the minimum heap  *
* property.                                                                    *
*******************************************************************************/
static void sift_down_root(heap* my_heap)
{
    heap_node* target = my_heap->state->table[0];
    void*      priority = target->priority;
    void*      min_child_priority;
    void*      tentative_priority;
    size_t     min_child_index;
    size_t     i;
    size_t     degree = my_heap->state->degree;
    size_t     index = 0;

    for (;;)
    {
        min_child_priority = priority;
        min_child_index = -1; /* Very large value to denote "no children". */
        compute_children_indices(my_heap, index);

        for (i = 0; i < degree; ++i)
        {
            if (my_heap->state->indices[i] == (size_t)-1)
            {
                break;
            }

            tentative_priority = my_heap->state->table[
                                 my_heap->state->indices[i]]
                               ->priority;

            if (my_heap->state->key_compare_function(min_child_priority,
                tentative_priority) > 0)
            {
                min_child_priority = tentative_priority;
                min_child_index = my_heap->state->indices[i];
            }
        }

        if (min_child_index == (size_t)-1)
        {
            my_heap->state->table[index] = target;
            target->index = index;
            return;
        }

        my_heap->state->table[index] = my_heap->state->table[min_child_index];
        my_heap->state->table[index]->index = index;

        index = min_child_index;
    }
}

/*******************************************************************************
* Makes sure that the heap has more room for new elements.                     *
*******************************************************************************/
static bool ensure_capacity_before_add(heap* my_heap)
{
    heap_node** new_table;
    size_t      new_capacity;
    size_t      i;

    if (my_heap->state->size < my_heap->state->capacity)
    {
        return true;
    }

    new_capacity = 3 * my_heap->state->capacity / 2;
    new_table = malloc(sizeof(heap_node*) * new_capacity);

    if (!new_table) return false;

    for (i = 0; i < my_heap->state->size; ++i)
    {
        new_table[i] = my_heap->state->table[i];
    }

    free(my_heap->state->table);
    my_heap->state->table = new_table;
    my_heap->state->capacity = new_capacity;
    return true;
}

bool heap_add(heap* my_heap, void* element, void* priority)
{
    heap_node* node;

    if (!my_heap)
    {
        return false;
    }

    /* Already in the heap? */
    if (unordered_map_contains_key(my_heap->state->node_map, element))
    {
        return false;
    }

    if (!ensure_capacity_before_add(my_heap))
    {
        return false;
    }

    node = heap_node_alloc(element, priority);

    if (!node)
    {
        return false;
    }

    node->index = my_heap->state->size;
    my_heap->state->table[my_heap->state->size] = node;

    unordered_map_put(my_heap->state->node_map, element, node);
    sift_up(my_heap, my_heap->state->size);
    my_heap->state->size++;
    return true;
}

bool heap_decrease_key(heap* my_heap, void* element, void* priority)
{
    heap_node* node;

    if (!my_heap)
    {
        return false;
    }

    if (!(node = unordered_map_get(my_heap->state->node_map, element)))
    {
        return false;
    }

    if (my_heap->state->key_compare_function(priority, node->priority) < 0)
    {
        node->priority = priority;
        sift_up(my_heap, node->index);
        return true;
    }

    return false;
}

bool heap_contains(heap* my_heap, void* element)
{
    if (!my_heap)
    {
        return false;
    }

    return unordered_map_contains_key(my_heap->state->node_map, element);
}

void* heap_extract_min(heap* my_heap)
{
    void* ret;
    heap_node* node;

    if (!my_heap || my_heap->state->size == 0)
    {
        return NULL;
    }

    node = my_heap->state->table[0];
    ret = node->element;
    my_heap->state->size--;
    my_heap->state->table[0] = my_heap->state->table[my_heap->state->size];
    unordered_map_remove(my_heap->state->node_map, ret);
    sift_down_root(my_heap);
    free(node);
    return ret;
}

void* heap_min(heap* my_heap)
{
    if (!my_heap || my_heap->state->size == 0)
    {
        return NULL;
    }

    return my_heap->state->table[0]->element;
}

int heap_size(heap* my_heap)
{
    return my_heap ? my_heap->state->size : -1;
}

void heap_clear(heap* my_heap)
{
    size_t i;

    if (!my_heap)
    {
        return;
    }

    unordered_map_clear(my_heap->state->node_map);

    for (i = 0; i < my_heap->state->size; ++i)
    {
        free(my_heap->state->table[i]);
    }

    my_heap->state->size = 0;
}

void heap_free(heap* my_heap)
{
    if (!my_heap)
    {
        return;
    }

    heap_clear(my_heap);
    unordered_map_free(my_heap->state->node_map);
    free(my_heap->state->indices);
    free(my_heap->state->table);
}