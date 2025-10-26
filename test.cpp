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
        if (list_size(graph[v]) != 0)
        {
            iterator_t x = list_head(graph[v]);
            while (IS_CORRECT(x))
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
    }
    measure_end("dejkstra on graph");
    
    not_optimize ^= dst[GraphNodes - 1];
}


const int Allocations = 10000;


/* allocator */
void test2()
{
    srand(179);

    vector<int> arr(2 * Allocations);
    for (int i = 0; i < Allocations; ++i)
    {
        arr[2 * i] = i;
        arr[2 * i + 1] = ~i;
    }
    std::mt19937 g(179);
    std::shuffle(arr.begin(), arr.end(), g);

    list_t *lst = list_create(Allocations / 20);
    for (int i = 0; i < Allocations; ++i)
    {
        if (i > 0)
        {
            list_insert();
        }
    }
    
    not_optimize ^= dst[GraphNodes - 1];
}



int main() 
{
    
    test1();
    
    fprintf(stderr, "not_optimize: %d\n", not_optimize);
    return 0;
}
