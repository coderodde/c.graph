#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "dijkstra.h"
#include "directed_graph_node.h"
#include "weight_function.h"
#include "utils.h"

#define ASSERT(CONDITION) assert(CONDITION, #CONDITION, __FILE__, __LINE__)

static bool assert(bool cond, char* err_msg, char* file_name, int line)
{
    if (!cond)
        fprintf(stderr,
            "'%s' is not true in file '%s' at line %d.\n",
            err_msg,
            file_name,
            line);

    return cond;
}

static void test_directed_graph_node_correctness()
{
    directed_graph_node* p_node_a;
    directed_graph_node* p_node_b;
    directed_graph_node* p_node_c;
    directed_graph_node* p_node_d;

    p_node_a = directed_graph_node_alloc("Node A");
    p_node_b = directed_graph_node_alloc("Node B");
    p_node_c = directed_graph_node_alloc("Node C");
    p_node_d = directed_graph_node_alloc("Node D");

    ASSERT(strcmp(directed_graph_node_to_string(p_node_a),
        "[directed_graph_node_t: id = Node A]") == 0);

    ASSERT(strcmp(directed_graph_node_to_string(p_node_b),
        "[directed_graph_node_t: id = Node B]") == 0);

    ASSERT(strcmp(directed_graph_node_to_string(p_node_c),
        "[directed_graph_node_t: id = Node C]") == 0);

    ASSERT(strcmp(directed_graph_node_to_string(p_node_d),
        "[directed_graph_node_t: id = Node D]") == 0);


    ASSERT(directed_graph_node_has_child(p_node_a, p_node_a) == false);
    ASSERT(directed_graph_node_has_child(p_node_a, p_node_b) == false);
    ASSERT(directed_graph_node_has_child(p_node_a, p_node_c) == false);
    ASSERT(directed_graph_node_has_child(p_node_a, p_node_d) == false);

    ASSERT(directed_graph_node_has_child(p_node_b, p_node_a) == false);
    ASSERT(directed_graph_node_has_child(p_node_b, p_node_b) == false);
    ASSERT(directed_graph_node_has_child(p_node_b, p_node_c) == false);
    ASSERT(directed_graph_node_has_child(p_node_b, p_node_d) == false);

    ASSERT(directed_graph_node_has_child(p_node_c, p_node_a) == false);
    ASSERT(directed_graph_node_has_child(p_node_c, p_node_b) == false);
    ASSERT(directed_graph_node_has_child(p_node_c, p_node_c) == false);
    ASSERT(directed_graph_node_has_child(p_node_c, p_node_d) == false);

    ASSERT(directed_graph_node_has_child(p_node_d, p_node_a) == false);
    ASSERT(directed_graph_node_has_child(p_node_d, p_node_b) == false);
    ASSERT(directed_graph_node_has_child(p_node_d, p_node_c) == false);
    ASSERT(directed_graph_node_has_child(p_node_d, p_node_d) == false);

    ASSERT(directed_graph_node_add_arc(p_node_a, p_node_b));
    ASSERT(directed_graph_node_has_child(p_node_a, p_node_b));
    ASSERT(directed_graph_node_has_child(p_node_b, p_node_a) == false);
    ASSERT(directed_graph_node_add_arc(p_node_b, p_node_a));
    ASSERT(directed_graph_node_has_child(p_node_a, p_node_b));
    ASSERT(directed_graph_node_has_child(p_node_b, p_node_a));
    /* a <-> b */
    ASSERT(directed_graph_node_has_child(p_node_a, p_node_a) == false);
    ASSERT(directed_graph_node_has_child(p_node_b, p_node_b) == false);

    ASSERT(directed_graph_node_add_arc(p_node_a, p_node_a));
    /* c a <-> b */
    ASSERT(directed_graph_node_has_child(p_node_a, p_node_a));
    ASSERT(directed_graph_node_has_child(p_node_b, p_node_b) == false);

    /* c a <-> b o*/
    ASSERT(directed_graph_node_add_arc(p_node_b, p_node_b));

    ASSERT(directed_graph_node_has_child(p_node_a, p_node_a));
    ASSERT(directed_graph_node_has_child(p_node_b, p_node_b));

    /* c a <-> b o -> d */
    ASSERT(directed_graph_node_add_arc(p_node_b, p_node_d));
    directed_graph_node_clear(p_node_a);

    ASSERT(directed_graph_node_has_child(p_node_a, p_node_a) == false);
    ASSERT(directed_graph_node_has_child(p_node_a, p_node_b) == false);
    ASSERT(directed_graph_node_has_child(p_node_a, p_node_c) == false);
    ASSERT(directed_graph_node_has_child(p_node_a, p_node_d) == false);
    ASSERT(directed_graph_node_has_child(p_node_b, p_node_d));
}

static void test_weight_function_correctness()
{
    directed_graph_weight_function* p_weight_function;

    directed_graph_node* p_node_a;
    directed_graph_node* p_node_b;
    directed_graph_node* p_node_c;
    directed_graph_node* p_node_d;

    double weight;

    p_node_a = directed_graph_node_alloc("Node A");
    p_node_b = directed_graph_node_alloc("Node B");
    p_node_c = directed_graph_node_alloc("Node C");
    p_node_d = directed_graph_node_alloc("Node D");

    ASSERT(p_weight_function =
        directed_graph_weight_function_alloc(hash_function,
            equals_function));

    ASSERT(directed_graph_weight_function_get(p_weight_function,
        p_node_a,
        p_node_c) == NULL);

    weight = 2.0;

    ASSERT(directed_graph_weight_function_put(p_weight_function,
        p_node_a,
        p_node_c,
        weight));

    ASSERT(*directed_graph_weight_function_get(p_weight_function,
        p_node_a,
        p_node_c) == 2.0);

    weight = 4.5;

    ASSERT(directed_graph_weight_function_put(p_weight_function,
        p_node_c,
        p_node_a,
        weight));

    ASSERT(*directed_graph_weight_function_get(p_weight_function,
        p_node_c,
        p_node_a) == 4.5);

    weight = 7.5;

    ASSERT(directed_graph_weight_function_put(p_weight_function,
        p_node_a,
        p_node_a,
        weight));

    ASSERT(*directed_graph_weight_function_get(p_weight_function,
        p_node_a,
        p_node_a) == 7.5);
    ASSERT(directed_graph_weight_function_get(p_weight_function,
        p_node_c,
        p_node_c) == NULL);

    directed_graph_weight_function_free(p_weight_function);
}

static void test_dijkstra_correctness()
{
    directed_graph_node* p_node_a;
    directed_graph_node* p_node_b;
    directed_graph_node* p_node_c;
    directed_graph_node* p_node_d;
    directed_graph_node* p_node_e;
    directed_graph_node* p_node_s;
    directed_graph_node* p_node_t;

    directed_graph_weight_function* p_weight_function;
    list* p_path;

    p_node_a = directed_graph_node_alloc("A");
    p_node_b = directed_graph_node_alloc("B");
    p_node_c = directed_graph_node_alloc("C");
    p_node_d = directed_graph_node_alloc("D");
    p_node_e = directed_graph_node_alloc("E");
    p_node_s = directed_graph_node_alloc("Source");
    p_node_t = directed_graph_node_alloc("Target");

    p_weight_function = directed_graph_weight_function_alloc(hash_function,
        equals_function);

    directed_graph_node_add_arc(p_node_s, p_node_a);
    directed_graph_weight_function_put(p_weight_function,
        p_node_s,
        p_node_a,
        1.0);

    directed_graph_node_add_arc(p_node_a, p_node_b);
    directed_graph_weight_function_put(p_weight_function,
        p_node_a,
        p_node_b,
        2.0);

    directed_graph_node_add_arc(p_node_b, p_node_c);
    directed_graph_weight_function_put(p_weight_function,
        p_node_b,
        p_node_c,
        3.0);

    directed_graph_node_add_arc(p_node_c, p_node_t);
    directed_graph_weight_function_put(p_weight_function,
        p_node_c,
        p_node_t,
        16.0);

    directed_graph_node_add_arc(p_node_s, p_node_d);
    directed_graph_weight_function_put(p_weight_function,
        p_node_s,
        p_node_d,
        11.0);

    directed_graph_node_add_arc(p_node_d, p_node_e);
    directed_graph_weight_function_put(p_weight_function,
        p_node_d,
        p_node_e,
        5.0);

    directed_graph_node_add_arc(p_node_e, p_node_t);
    directed_graph_weight_function_put(p_weight_function,
        p_node_e,
        p_node_t,
        6.0);

    directed_graph_node_add_arc(p_node_c, p_node_d);
    directed_graph_weight_function_put(p_weight_function,
        p_node_c,
        p_node_d,
        4.0);

    p_path = dijkstra(p_node_s, p_node_t, p_weight_function);

    ASSERT(list_size(p_path) == 7);
    ASSERT(list_get(p_path, 0) == p_node_s);
    ASSERT(list_get(p_path, 1) == p_node_a);
    ASSERT(list_get(p_path, 2) == p_node_b);
    ASSERT(list_get(p_path, 3) == p_node_c);
    ASSERT(list_get(p_path, 4) == p_node_d);
    ASSERT(list_get(p_path, 5) == p_node_e);
    ASSERT(list_get(p_path, 6) == p_node_t);
}

static const size_t NODES = 20000;
static const size_t EDGES = 20000 * 9;
static const double MAXX = 10000.0;
static const double MAXY = 10000.0;
static const double MAXZ = 200.0;
static const double MAX_DISTANCE = 800.0;

int main(int argc, char** argv) {
    graph_data* p_data;
    clock_t       c;
    int           seed = time(NULL);
    double        duration;
    list*       p_path;
    size_t        i;

    directed_graph_node* p_source;
    directed_graph_node* p_target;

    printf("Seed: %d\n", seed);
    srand(seed);

    test_directed_graph_node_correctness();
    test_weight_function_correctness();
    test_dijkstra_correctness();
    //test_bidirectional_dijkstra_correctness();

    c = clock();
    p_data = create_random_graph(NODES, EDGES, MAXX, MAXY, MAXZ, MAX_DISTANCE);

    duration = ((double)clock() - c);
    printf("Built the graph in %f seconds.\n", duration / CLOCKS_PER_SEC);

    p_source = choose(p_data->p_node_array, NODES);
    p_target = choose(p_data->p_node_array, NODES);

    printf("Source: %s\n", directed_graph_node_to_string(p_source));
    printf("Target: %s\n", directed_graph_node_to_string(p_target));

    /**** DIJKSTRA'S ALGORITHM ****/
    c = clock();

    p_path = dijkstra(p_source, p_target, p_data->p_weight_function);

    duration = ((double)clock() - c);

    printf("Dijkstra's algorithm in %f seconds.\n", duration / CLOCKS_PER_SEC);
    printf("Path:\n");

    for (i = 0; i < list_size(p_path); ++i)
    {
        puts(directed_graph_node_to_string(list_get(p_path, i)));
    }

    printf("Path is a valid path: %d\n", is_valid_path(p_path));
    printf("Path cost: %f\n",
        compute_path_cost(p_path, p_data->p_weight_function));

    /**** ASTAR ALGORITHM ****/
    /*c = clock();

    p_path = astar(p_source,
        p_target,
        p_data->p_weight_function,
        p_data->p_point_map);

    duration = ((double)clock() - c);

    printf("A* algorithm in %f seconds.\n", duration / CLOCKS_PER_SEC);
    printf("Path:\n");

    for (i = 0; i < list_t_size(p_path); ++i)
    {
        puts(directed_graph_node_t_to_string(list_t_get(p_path, i)));
    }

    printf("Path is a valid path: %d\n", is_valid_path(p_path));
    printf("Path cost: %f\n",
        compute_path_cost(p_path, p_data->p_weight_function));*/

    /**** BIDIRECTIONAL DIJKSTRA'S ALGORITHM ****//*
    c = clock();

    p_path = bidirectional_dijkstra(p_source,
        p_target,
        p_data->p_weight_function);

    duration = ((double)clock() - c);

    printf("Bidirectional Dijkstra's algorithm in %f seconds.\n",
        duration / CLOCKS_PER_SEC);

    printf("Path:\n");

    for (i = 0; i < list_t_size(p_path); ++i)
    {
        puts(directed_graph_node_t_to_string(list_t_get(p_path, i)));
    }

    printf("Path is a valid path: %d\n", is_valid_path(p_path));
    printf("Path cost: %f\n",
        compute_path_cost(p_path, p_data->p_weight_function));
        */
    /**** BIDIRECTIONAL ASTAR ALGORITHM ****//*
    c = clock();

    p_path = bidirectional_astar(p_source,
        p_target,
        p_data->p_weight_function,
        p_data->p_point_map);

    duration = ((double)clock() - c);

    printf("Bidirectional A* algorithm in %f seconds.\n",
        duration / CLOCKS_PER_SEC);

    printf("Path:\n");

    for (i = 0; i < list_t_size(p_path); ++i)
    {
        puts(directed_graph_node_t_to_string(list_t_get(p_path, i)));
    }

    printf("Path is a valid path: %d\n", is_valid_path(p_path));
    printf("Path cost: %f\n",
        compute_path_cost(p_path, p_data->p_weight_function));*/

    return (EXIT_SUCCESS);
}
