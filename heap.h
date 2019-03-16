#ifndef HEAP_H
#define	HEAP_H

#include <stdbool.h>
#include <stdlib.h>

#ifdef	__cplusplus
extern "C" {
#endif

    typedef struct heap {
        struct heap_state* state;
    } heap;

    /***************************************************************************
    * Allocates a new, empty heap with given degree.                           *
    ***************************************************************************/
    heap* heap_alloc(size_t   degree,
                     size_t   initial_capacity,
                     float    load_factor,
                     size_t(*hash_function)(void*),
                     bool(*equals_function)(void*, void*),
                     int(*priority_compare_function)(void*, void*));

    /***************************************************************************
    * Adds a new element and its priority to the heap only if it is not        *
    * already present.                                                         *
    ***************************************************************************/
    bool heap_add(heap* heap, void* element, void* priority);

    /***************************************************************************
    * Attempts to assign a higher priority to the element. Return true only    *
    * if the structure of the heap changed due to this call.                   *
    ***************************************************************************/
    bool heap_decrease_key(heap* heap, void* element, void* priority);

    /***************************************************************************
    * Return true only if the element is in the heap.                          *
    ***************************************************************************/
    bool heap_contains(heap* heap, void* element);

    /***************************************************************************
    * Removes the highest priority element and returns it.                     *
    ***************************************************************************/
    void* heap_extract_min(heap* heap);

    /***************************************************************************
    * Returns the highest priority element without removing it.                *
    ***************************************************************************/
    void* heap_min(heap* heap);

    /***************************************************************************
    * Returns the size of this heap.                                           *
    ***************************************************************************/
    int heap_size(heap* heap);

    /***************************************************************************
    * Drops all the contents of the heap. Only internal structures are         *
    * deallocated; the user is responsible for memory-managing the contents.   *
    ***************************************************************************/
    void heap_clear(heap* heap);

    /***************************************************************************
    * Checks that the heap maintains the min-heap property.                    *
    ***************************************************************************/
    bool heap_is_healthy(heap* heap);

    /***************************************************************************
    * Deallocates the entire heap with its internal structures. The client     *
    * programmer must, however, memory-manage the contents.                    *
    ***************************************************************************/
    void heap_free(heap* heap);

#ifdef	__cplusplus
}
#endif

#endif	/* HEAP_H */