#include "windows.h"
#include "stdio.h"

#include "list.h"

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
    fprintf(stderr, "at test %s used %.2lf ms.\n", miliseconds);
}



void test1()
{
    
}



int main() 
{
    measure_start();
    
    measure_end("");
    return 0;
}
