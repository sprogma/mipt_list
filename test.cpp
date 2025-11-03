#include "windows.h"
#include "stdio.h"
#include "assert.h"

#include "mylist.h"

#include <map>
#include <set>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <random>
#include <algorithm>

LARGE_INTEGER frequency;
LARGE_INTEGER start_time, end_time;


void measure_start()
{
    QueryPerformanceCounter(&start_time);
}

void measure_end(const char *name)
{
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&end_time);
    
    double miliseconds = (double)(end_time.QuadPart - start_time.QuadPart) * 1e3 / frequency.QuadPart;

    printf("%.10lf\n", miliseconds);
    fprintf(stderr, "at test %s used %.2lf ms.\n", name, miliseconds);
}

int not_optimize = 0;


using namespace std;


void test_behaviour()
{
    #ifndef TEST_CPP_REALIZATION
    list_t *lst = list_create(0);
    int res = 0;

    res = list_insert(lst, list_head(lst), 1);
    // printf("insert at %d\n", res);
    res = list_insert(lst, list_head(lst), 2);
    // printf("insert at %d\n", res);
    res = list_insert(lst, list_head(lst), 3);
    // printf("insert at %d\n", res);

    res = list_get(lst, list_head(lst));
    // printf("READ %d\n", res);
    assert(res == 3);
    list_remove(lst, list_head(lst));

    res = list_get(lst, list_head(lst));
    // printf("READ %d\n", res);
    assert(res == 2);
    list_remove(lst, list_head(lst));
    
    res = list_get(lst, list_head(lst));
    // printf("READ %d\n", res);
    assert(res == 1);
    list_remove(lst, list_head(lst));

    assert(list_size(lst) == 0);

    list_free(lst);

    /* build list + clear list */
    lst = list_create(0);
    #define N 3
    for (int i = 0, t = 0; t < N; i = (i + 7) % N, t++)
    {
        // printf("------------------- Insert %d at begin!\n", i);
        list_insert(lst, list_head(lst), i);
    }
    for (int i = 0; i < N; ++i)
    {
        int t = 0;
        if (i % 2 == 0)
        {
            // printf("         ------------------------------ forward\n");
            iterator_t it = list_head(lst);
            while (is_correct(it))
            {
                if (list_get(lst, it) == i)
                {
                    // printf("------------------- Find %d at position %d!\n", i, t);
                    list_remove(lst, it);
                    break;
                }
                t++;
                // printf("-------------------- was it = %d [%d]\n", it, list_get(lst, it));
                it = list_next(lst, it);
                // printf("-------------------- new it = %d [?]\n", it);
            }
        }
        else
        {
            // printf("         ------------------------------ backwards\n");
            iterator_t it = list_tail(lst);
            while (is_correct(it))
            {
                if (list_get(lst, it) == i)
                {
                    // printf("------------------- Find %d at position %d!\n", i, t);
                    list_remove(lst, it);
                    break;
                }
                t++;
                // printf("-------------------- was it = %d [%d]\n", it, list_get(lst, it));
                it = list_prev(lst, it);
                // printf("-------------------- new it = %d [?]\n", it);
            } 
        }
    }
    #undef N

    // printf("At end size == %d\n", list_size(lst));

    assert(list_size(lst) == 0);

    list_free(lst);
    
    
    #endif
}


const int GraphNodes = 5000;


/* graph representation */
void test1()
{
    srand(179);
    
    vector<list_t*> graph(GraphNodes);
    vector<int> dst(GraphNodes, GraphNodes + 1000); // GraphNodes + 1 is max distance
    set<pair<int, int>> q;
    q.insert({0, 0});
    dst[0] = 0;
    
    /* generate graph */
    measure_start();
    for (int i = 0; i < (int)graph.size(); ++i)
    {
        graph[i] = list_create(GraphNodes / 10);
    }
    for (int i = 0; i < 5 + GraphNodes * GraphNodes / 10; ++i)
    {
        int a = rand() % GraphNodes, b = rand() % GraphNodes;
        list_insert(graph[a], list_head(graph[a]), b);
        list_insert(graph[b], list_head(graph[b]), a);
    }
    while (!q.empty())
    {
        int v = q.begin()->second;
        q.erase(q.begin());
        iterator_t x = list_head(graph[v]);
        while (is_correct(x))
        {
            int value = list_get(graph[v], x);
            if (dst[value] > dst[v] + 1)
            {
                q.erase({dst[value], value});
                dst[value] = dst[v] + 1;
                q.insert({dst[value], value});
            }
            x = list_next(graph[v], x);
        }
    }
    for (int i = 0; i < (int)graph.size(); ++i)
    {
        list_free(graph[i]);
    }
    measure_end("dejkstra on graph");
    
    not_optimize ^= dst[GraphNodes - 1];
}


const int BfsGraphNodes = 5000;


/* graph representation */
void test2()
{
    srand(179);
    
    vector<list_t*> graph(BfsGraphNodes);
    vector<int> dst(BfsGraphNodes, -1);
    
    /* generate graph */
    measure_start();
    for (int i = 0; i < (int)graph.size(); ++i)
    {
        graph[i] = list_create(BfsGraphNodes / 10);
    }
    for (int i = 0; i < 5 + BfsGraphNodes * BfsGraphNodes / 10; ++i)
    {
        int a = rand() % BfsGraphNodes, b = rand() % BfsGraphNodes;
        list_insert(graph[a], list_head(graph[a]), b);
        list_insert(graph[b], list_head(graph[b]), a);
    }
    
    list_t *queue = list_create(BfsGraphNodes);
    list_insert(queue, list_head(queue), 0);
    dst[0] = 0;
    
    
    while (list_size(queue) > 0)
    {
        int v = list_get(queue, list_head(queue));
        list_remove(queue, list_head(queue));
        iterator_t x = list_head(graph[v]);
        while (is_correct(x))
        {
            int value = list_get(graph[v], x);
            if (dst[value] == -1)
            {        
                dst[value] = dst[v] + 1;
                list_insert(queue, list_tail(queue), value);
            }
            x = list_next(graph[v], x);
        }
    }
    for (int i = 0; i < (int)graph.size(); ++i)
    {
        list_free(graph[i]);
    }
    list_free(queue);
    measure_end("BFS on graph");
    
    not_optimize ^= dst[BfsGraphNodes - 1];
}


const int ArrayReverseSize = 10000000;


/* graph representation */
void test31()
{
    srand(179);

    measure_start();
    list_t *lst = list_create(0);
    for (int i = 0; i < ArrayReverseSize; ++i)
    {
        list_insert(lst, list_head(lst), i);
    }
    for (int i = 0; i < ArrayReverseSize; ++i)
    {
        iterator_t h = list_head(lst);
        int v = list_get(lst, h);
        if (v != ArrayReverseSize - i - 1)
        {
            fprintf(stderr, "ERROR: list behaviour is broken. get %d instead of %d [i=%d]\n", v, ArrayReverseSize - i - 1, i);
            return;
        }
        list_remove(lst, h);
    }
    list_free(lst);
    measure_end("insert and remove big array, as stack, RESERVE(0%)");
    // not_optimize ^= lst;
}

/* graph representation */
void test32()
{
    srand(179);

    measure_start();
    list_t *lst = list_create(ArrayReverseSize);
    for (int i = 0; i < ArrayReverseSize; ++i)
    {
        list_insert(lst, list_head(lst), i);
    }
    for (int i = 0; i < ArrayReverseSize; ++i)
    {
        iterator_t h = list_head(lst);
        int v = list_get(lst, h);
        if (v != ArrayReverseSize - i - 1)
        {
            fprintf(stderr, "ERROR: list behaviour is broken.\n");
            return;
        }
        list_remove(lst, h);
    }
    list_free(lst);
    measure_end("insert and remove big array, as stack, RESERVE(100%)");
    // not_optimize ^= lst;
}


const int ArrayReadSize = 10000000; // 3e7


list_t *build_nonlinear_list(int size)
{ 
    list_t *lst = list_create(0);

    for (int i = 0; i < size; ++i)
    {
        list_insert(lst, list_head(lst), i);
    }
    for (int t = 0; t < 5; ++t)
    {
        vector<int> del;
        del.reserve(size / 2);
        iterator_t it = list_head(lst);
        while (is_correct(it))
        {
            if (rand() % 2 == 0)
            {
                del.push_back(list_get(lst, it));
                it = list_remove(lst, it);
            }
        }
        for (int i : del)
        {
            list_insert(lst, list_head(lst), i);
        }
    }
    // for (int i = 0; i < size; ++i)
    // {
    //     #ifdef TEST_CPP_REALIZATION
    //     if (i % 2 == 0)
    //     {
    //         list_insert(lst, list_head(lst), i);
    //     }
    //     else
    //     {
    //         list_insert(lst, list_tail(lst), i);
    //     }
    //     #else
    //     if (i <= 1)
    //     {
    //         list_insert(lst, list_tail(lst), i);
    //     }
    //     else
    //     {
    //         list_insert(lst, 2 + rand() % i, i);
    //     }
    //     #endif
    // }
    return lst;
}

void print_avr_jump_size(list_t *lst)
{
    #ifndef TEST_CPP_REALIZATION
    iterator_t h = list_head(lst);
    double sum = 0;
    while (is_correct(h))
    {
        int nh = list_next(lst, h);
        if (is_correct(nh))
        {
            sum += fabs(h - nh);
        }
        h = nh;
    }
    fprintf(stderr, "! average jump size is: %f\n", sum / list_size(lst));
    #endif
}

/* graph representation */
void test4()
{
    srand(179);
    list_t *lst = build_nonlinear_list(ArrayReadSize);


    print_avr_jump_size(lst);

    
    measure_start();
    {
        iterator_t h = list_head(lst);
        int x = ArrayReadSize;
        int res = 0;
        while (is_correct(h))
        {
            int v = list_get(lst, h);
            res += v * v * v;
            h = list_next(lst, h);
            --x;
        }
        if (x != 0)
        {
            fprintf(stderr, "ERROR: list behaviour is broken.\n");
            return;
        }
    }
    measure_end("simlpy read big shuffled array");
    
    list_free(lst);
    // not_optimize ^= lst;
}


/* graph representation */
void test5()
{
    srand(179);
    
    list_t *lst = build_nonlinear_list(ArrayReadSize);
    
    measure_start();
    list_optimize(lst);
    measure_end("optimization used time");
    
    print_avr_jump_size(lst);
    
    measure_start();
    {
        iterator_t h = list_head(lst);
        int x = ArrayReadSize;
        int res = 0;
        while (is_correct(h))
        {
            int v = list_get(lst, h);
            res += v * v * v;
            h = list_next(lst, h);
            --x;
        }
        if (x != 0)
        {
            fprintf(stderr, "ERROR: list behaviour is broken. [x=%d]\n", x);
            return;
        }
    }
    measure_end("read big shuffled array, after optimization");
    
    list_free(lst);
    // not_optimize ^= lst;
}



int main() 
{
    
    test_behaviour();
    test1();
    test2();
    test31();
    test32();
    test4();
    test5();
    
    fprintf(stderr, "not_optimize: %d\n", not_optimize);
    return 0;
}




