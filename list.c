#include "list.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stddef.h>

typedef struct list_state {
    void** storage;
    int size; /* TODO: change to size_t? */
    int capacity;
    int head;
    int mask;
} list_state;
/*
typedef struct list {
    list_state* state;
} list;*/

static const int MINIMUM_CAPACITY = 16;
/*
static size_t max(size_t a, size_t b)
{
    return a < b ? b : a;
}*/

static int fix_initial_capacity(int initial_capacity)
{
    int ret = 1;

    initial_capacity = max(initial_capacity, MINIMUM_CAPACITY);

    while (ret < initial_capacity)
    {
        ret <<= 1;
    }

    return ret;
}

list* list_alloc(size_t initial_capacity)
{
    list* my_list = malloc(sizeof(*my_list));
    list_state* my_list_state;

    if (!my_list)
    {
        return NULL;
    }

    my_list_state = malloc(sizeof(*my_list_state));

    if (!my_list_state)
    {
        free(my_list);
        return NULL;
    }

    my_list->state = my_list_state;
    initial_capacity = fix_initial_capacity(initial_capacity);

    my_list->state->storage = malloc(sizeof(void*) * initial_capacity);

    if (!my_list->state->storage)
    {
        free(my_list);
        return NULL;
    }

    my_list->state->capacity = initial_capacity;
    my_list->state->mask = initial_capacity - 1;
    my_list->state->head = 0;
    my_list->state->size = 0;

    return my_list;
}

static bool ensure_capacity_before_add(list* my_list)
{
    void** new_table;
    size_t i;
    size_t new_capacity;

    if (my_list->state->size < my_list->state->capacity)
    {
        return true;
    }

    new_capacity = 2 * my_list->state->capacity;
    new_table    = malloc(sizeof(void*) * new_capacity);

    if (!new_table)
    {
        return false;
    }

    for (i = 0; i < my_list->state->size; ++i)
    {
        new_table[i] = 
            my_list->state->storage[
                (my_list->state->head + i) & my_list->state->mask];
    }

    free(my_list->state->storage);

    my_list->state->storage  = new_table;
    my_list->state->capacity = new_capacity;
    my_list->state->mask     = new_capacity - 1;
    my_list->state->head     = 0;

    return true;
}

bool list_push_front(list* my_list, void* element)
{
    if (!my_list)
    {
        return false;
    }

    if (!ensure_capacity_before_add(my_list))
    {
        return false;
    }

    my_list->state->head = 
        (my_list->state->head - 1) & my_list->state->mask;

    my_list->state->storage[my_list->state->head] = element;
    my_list->state->size++;

    return true;
}

bool list_push_back(list* my_list, void* element)
{
    if (!my_list)
    {
        return false;
    }

    if (!ensure_capacity_before_add(my_list))
    {
        return false;
    }

    my_list->state->storage[(my_list->state->head + 
                             my_list->state->size) & 
                             my_list->state->mask] = element;
    my_list->state->size++;
    return true;
}

bool list_insert(list* my_list, size_t index, void* element)
{
    size_t elements_before;
    size_t elements_after;
    size_t i;
    size_t head;
    size_t mask;
    size_t size;

    if (!my_list)
    {
        return false;
    }

    if (!ensure_capacity_before_add(my_list))
    {
        return false;
    }

    if (index > my_list->state->size)
    {
        return false;
    }

    elements_before = index;
    elements_after = my_list->state->size - index;
    head = my_list->state->head;
    mask = my_list->state->mask;
    size = my_list->state->size;

    if (elements_before < elements_after)
    {
        /* Move preceding elements one position to the left. */
        for (i = 0; i < elements_before; ++i)
        {
            my_list->state->storage[(head + i - 1) & mask] =
                my_list->state->storage[(head + i) & mask];
        }

        head = (head - 1) & mask;
        my_list->state->storage[(head + index) & mask] = element;
        my_list->state->head = head;
    }
    else
    {
        /* Move the following elements one position to the right. */
        for (i = 0; i < elements_after; ++i)
        {
            my_list->state->storage[(head + size - i) & mask] =
                my_list->state->storage[(head + size - i - 1) & mask];
        }

        my_list->state->storage[(head + index) & mask] = element;
    }

    my_list->state->size++;
    return true;
}

size_t list_size(list* my_list)
{
    return my_list ? my_list->state->size : 0;
}

void* list_get(list* my_list, size_t index)
{
    if (!my_list)
    {
        return NULL;
    }

    if (index >= my_list->state->size)
    {
        return NULL;
    }

    return my_list->state->storage[
        (my_list->state->head + index) & 
         my_list->state->mask];
}

void* list_set(list* my_list, size_t index, void* new_value)
{
    void* old_value;

    if (!my_list)
    {
        return NULL;
    }

    if (index >= my_list->state->size)
    {
        return NULL;
    }

    old_value = my_list->state->storage[
        (my_list->state->head + index) & 
         my_list->state->mask];

    my_list->state->storage[
        (my_list->state->head + index) & 
         my_list->state->mask] = new_value;

    return old_value;
}

void* list_pop_front(list* my_list)
{
    void* front;

    if (!my_list)
    {
        return NULL;
    }

    if (my_list->state->size == 0)
    {
        return NULL;
    }

    front = my_list->state->storage[my_list->state->head];
    my_list->state->head = (my_list->state->head + 1) & my_list->state->mask;
    my_list->state->size--;
    return front;
}

void* list_pop_back(list* my_list)
{
    void* back;

    if (!my_list)
    {
        return NULL;
    }

    if (my_list->state->size == 0)
    {
        return NULL;
    }

    back = my_list->state->storage[
        (my_list->state->head + my_list->state->size - 1) &
         my_list->state->mask];

    my_list->state->size--;
    return back;
}

void* list_remove_at(list* my_list, size_t index)
{
    void* value;
    size_t head;
    size_t mask;
    size_t elements_before;
    size_t elements_after;
    size_t i;
    size_t j;

    if (!my_list)
    {
        return NULL;
    }

    if (index >= my_list->state->size)
    {
        return NULL;
    }

    head = my_list->state->head;
    mask = my_list->state->mask;

    value = my_list->state->storage[(head + index) & mask];

    elements_before = index;
    elements_after = my_list->state->size - index - 1;

    if (elements_before < elements_after)
    {
        /* Move the preceding elements one position to the right. */
        for (j = elements_before; j > 0; --j)
        {
            my_list->state->storage[(head + j) & mask] =
                my_list->state->storage[(head + j - 1) & mask];
        }

        my_list->state->head = (head + 1) & mask;
    }
    else
    {
        /* Move the following elements one position to the left. */
        for (i = 0; i < elements_after; ++i)
        {
            my_list->state->storage[(head + index + i) & mask] =
                my_list->state->storage[(head + index + i + 1) & mask];
        }
    }

    my_list->state->size--;
    return value;
}

bool list_contains(list* my_list,
    void* element,
    bool(*equals_function)(void*, void*))
{
    size_t i;

    if (!my_list)
    {
        return false;
    }

    if (!equals_function)
    {
        return false;
    }

    for (i = 0; i < my_list->state->size; ++i)
    {
        if (equals_function(element,
            my_list->state->storage[(my_list->state->head + i) &
            my_list->state->mask]))
        {
            return true;
        }
    }

    return false;
}

void list_clear(list* my_list)
{
    if (!my_list)
    {
        return;
    }

    my_list->state->head = 0;
    my_list->state->size = 0;
}

void list_free(list* my_list)
{
    if (!my_list)
    {
        return;
    }

    free(my_list->state->storage);
    free(my_list->state);
    free(my_list);
}