#include "unordered_set.h"
#include <stdbool.h>
#include <stdlib.h>

typedef struct unordered_set_entry {
    void*                       key;
    struct unordered_set_entry* chain_next;
    struct unordered_set_entry* prev;
    struct unordered_set_entry* next;
} unordered_set_entry;

typedef struct unordered_set_state {
    unordered_set_entry** table;
    unordered_set_entry*  head;
    unordered_set_entry*  tail;
    size_t(*hash_function)(void*);
    bool(*equals_function)(void*, void*);
    size_t                mod_count;
    size_t                table_capacity;
    size_t                size;
    size_t                mask;
    size_t                max_allowed_size;
    float                 load_factor;
} unordered_set_state;
/*
typedef struct unordered_set {
    unordered_set_state* state;
} unordered_set;*/

typedef struct unordered_set_iterator {
    unordered_set*       set;
    unordered_set_entry* next_entry;
    size_t               iterated_count;
    size_t               expected_mod_count;
} unordered_set_iterator;

static unordered_set_entry* unordered_set_entry_alloc(void* key)
{
    unordered_set_entry* entry = malloc(sizeof(*entry));

    if (!entry)
    {
        return NULL;
    }

    entry->key = key;
    entry->chain_next = NULL;
    entry->next = NULL;
    entry->prev = NULL;

    return entry;
}

static const float  MINIMUM_LOAD_FACTOR = 0.2f;
static const int MINIMUM_INITIAL_CAPACITY = 16;

static float maxf(float a, float b)
{
    return a < b ? b : a;
}

static int maxi(int a, int b)
{
    return a < b ? b : a;
}

/*******************************************************************************
* Makes sure that the load factor is no less than a minimum threshold.         *
*******************************************************************************/
static float fix_load_factor(float load_factor)
{
    return maxf(load_factor, MINIMUM_LOAD_FACTOR);
}

/*******************************************************************************
* Makes sure that the initial capacity is no less than a minimum allowed and   *
* is a power of two.                                                           *
*******************************************************************************/
static size_t fix_initial_capacity(size_t initial_capacity)
{
    size_t ret;

    initial_capacity = maxi(initial_capacity, MINIMUM_INITIAL_CAPACITY);
    ret = 1;

    while (ret < initial_capacity)
    {
        ret <<= 1;
    }

    return ret;
}

unordered_set* unordered_set_alloc(size_t initial_capacity,
    float load_factor,
    size_t(*hash_function)(void*),
    bool(*equals_function)(void*, void*))
{
    unordered_set* set;

    if (!hash_function || !equals_function)
    {
        return NULL;
    }

    set = malloc(sizeof(*set));

    if (!set)
    {
        return NULL;
    }

    set->state = malloc(sizeof(*set->state));
    load_factor = fix_load_factor(load_factor);
    initial_capacity = fix_initial_capacity(initial_capacity);

    set->state->load_factor = load_factor;
    set->state->table_capacity = initial_capacity;
    set->state->size = 0;
    set->state->mod_count = 0;
    set->state->head = NULL;
    set->state->tail = NULL;
    set->state->table = calloc(initial_capacity,
        sizeof(unordered_set_entry*));
    set->state->hash_function = hash_function;
    set->state->equals_function = equals_function;
    set->state->mask = initial_capacity - 1;
    set->state->max_allowed_size = (size_t)(initial_capacity * load_factor);

    return set;
}

static void ensure_capacity(unordered_set* set)
{
    size_t new_capacity;
    size_t new_mask;
    size_t index;
    unordered_set_entry*  entry;
    unordered_set_entry** new_table;

    if (set->state->size < set->state->max_allowed_size)
    {
        return;
    }

    new_capacity = 2 * set->state->table_capacity;
    new_mask = new_capacity - 1;
    new_table = calloc(new_capacity, sizeof(unordered_set_entry*));

    if (!new_table)
    {
        return;
    }

    /* Rehash the entries. */
    for (entry = set->state->head; entry; entry = entry->next)
    {
        index = set->state->hash_function(entry->key) & new_mask;
        entry->chain_next = new_table[index];
        new_table[index] = entry;
    }

    free(set->state->table);

    set->state->table = new_table;
    set->state->table_capacity = new_capacity;
    set->state->mask = new_mask;
    set->state->max_allowed_size = (size_t)(new_capacity * set->state->load_factor);
}

bool unordered_set_add(unordered_set* set, void* key)
{
    size_t index;
    size_t hash_value;
    unordered_set_entry* entry;

    if (!set)
    {
        return NULL;
    }

    hash_value = set->state->hash_function(key);
    index = hash_value & set->state->mask;

    for (entry = set->state->table[index]; entry; entry = entry->chain_next)
    {
        if (set->state->equals_function(entry->key, key))
        {
            return false;
        }
    }

    ensure_capacity(set);

    /* Recompute the index since it is possibly changed by 'ensure_capacity' */
    index = hash_value & set->state->mask;
    entry = unordered_set_entry_alloc(key);
    entry->chain_next = set->state->table[index];
    set->state->table[index] = entry;

    /* Link the new entry to the tail of the list. */
    if (!set->state->tail)
    {
        set->state->head = entry;
        set->state->tail = entry;
    }
    else
    {
        set->state->tail->next = entry;
        entry->prev = set->state->tail;
        set->state->tail = entry;
    }

    set->state->size++;
    set->state->mod_count++;

    return true;
}

bool unordered_set_contains(unordered_set* set, void* key)
{
    size_t index;
    unordered_set_entry* p_entry;

    if (!set)
    {
        return false;
    }

    index = set->state->hash_function(key) & set->state->mask;

    for (p_entry = set->state->table[index]; p_entry; p_entry = p_entry->chain_next)
    {
        if (set->state->equals_function(key, p_entry->key))
        {
            return true;
        }
    }

    return false;
}

bool unordered_set_remove(unordered_set* set, void* key)
{
    size_t index;
    unordered_set_entry* prev_entry;
    unordered_set_entry* current_entry;

    if (!set)
    {
        return false;
    }

    index = set->state->hash_function(key) & set->state->mask;

    prev_entry = NULL;

    for (current_entry = set->state->table[index];
        current_entry;
        current_entry = current_entry->chain_next)
    {
        if (set->state->equals_function(key, current_entry->key))
        {
            if (prev_entry)
            {
                /* Omit the 'p_current_entry' in the collision chain. */
                prev_entry->chain_next = current_entry->chain_next;
            }
            else
            {
                set->state->table[index] = current_entry->chain_next;
            }

            /* Unlink from the global iteration chain. */
            if (current_entry->prev)
            {
                current_entry->prev->next = current_entry->next;
            }
            else
            {
                set->state->head = current_entry->next;
            }

            if (current_entry->next)
            {
                current_entry->next->prev = current_entry->prev;
            }
            else
            {
                set->state->tail = current_entry->prev;
            }

            set->state->size--;
            set->state->mod_count++;
            free(current_entry);
            return true;
        }

        prev_entry = current_entry;
    }

    return false;
}

void unordered_set_clear(unordered_set* set)
{
    unordered_set_entry* entry;
    unordered_set_entry* next_entry;
    size_t index;

    if (!set)
    {
        return;
    }

    entry = set->state->head;

    while (entry)
    {
        index = set->state->hash_function(entry->key) & set->state->mask;
        next_entry = entry->next;
        free(entry);
        entry = next_entry;
        set->state->table[index] = NULL;
    }

    set->state->mod_count += set->state->size;
    set->state->size = 0;
    set->state->head = NULL;
    set->state->tail = NULL;
}

size_t unordered_set_size(unordered_set* set)
{
    return set ? set->state->size : 0;
}

bool unordered_set_is_healthy(unordered_set* set)
{
    size_t counter;
    unordered_set_entry* entry;

    if (!set)
    {
        return false;
    }

    counter = 0;
    entry = set->state->head;

    if (entry && entry->prev)
    {
        return false;
    }

    for (; entry; entry = entry->next)
    {
        counter++;
    }

    return counter == set->state->size;
}

void unordered_set_free(unordered_set* set)
{
    if (!set)
    {
        return;
    }

    unordered_set_clear(set);
    free(set->state->table);
    free(set);
}

unordered_set_iterator*
unordered_set_iterator_alloc(unordered_set* set)
{
    unordered_set_iterator* iterator;

    if (!set)
    {
        return NULL;
    }

    iterator = malloc(sizeof(*iterator));

    if (!iterator)
    {
        return NULL;
    }

    iterator->set = set;
    iterator->iterated_count = 0;
    iterator->next_entry = set->state->head;
    iterator->expected_mod_count = set->state->mod_count;

    return iterator;
}

size_t unordered_set_iterator_has_next(unordered_set_iterator* iterator)
{
    if (!iterator)
    {
        return 0;
    }

    if (unordered_set_iterator_is_disturbed(iterator))
    {
        return 0;
    }

    return iterator->set->state->size - iterator->iterated_count;
}

bool unordered_set_iterator_next(unordered_set_iterator* iterator,
    void** key_pointer)
{
    if (!iterator)
    {
        return false;
    }

    if (!iterator->next_entry)
    {
        return false;
    }

    if (unordered_set_iterator_is_disturbed(iterator))
    {
        return false;
    }

    *key_pointer = iterator->next_entry->key;
    iterator->iterated_count++;
    iterator->next_entry = iterator->next_entry->next;

    return true;
}

bool
unordered_set_iterator_is_disturbed(unordered_set_iterator* iterator)
{
    if (!iterator)
    {
        false;
    }

    return iterator->expected_mod_count != iterator->set->state->mod_count;
}

void unordered_set_iterator_free(unordered_set_iterator* iterator)
{
    if (!iterator)
    {
        return;
    }

    iterator->set = NULL;
    iterator->next_entry = NULL;
    free(iterator);
}
