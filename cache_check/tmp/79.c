#define __USE_MINGW_ANSI_STDIO 1
#include "immintrin.h"
#include "stdio.h"
#include "malloc.h"
#include "string.h"
#include "windows.h"

#define SIZE_MASK 2147483647LL
#define SIZE 2147483648LL

int main()
{
    char *array = malloc(SIZE);
    
    /* allocate array from system */
    for (long long i = 0; i < SIZE; i += 4096)
    {
        array[i] = 1;
    }

    /* clear cache */
    char *arr2 = malloc(256 * 1024 * 1024);
    memset(arr2, 0xCC, 256 * 1024 * 1024);
    memcpy(arr2 + 128 * 1024 * 1024, arr2, 128 * 1024 * 1024);
    free(arr2);

    {
        HANDLE hCurrentThread = GetCurrentThread();
        DWORD_PTR newAffinityMask = 0x00000001;
        SetThreadAffinityMask(hCurrentThread, newAffinityMask);
        
        /* measure loop with fetching */
        LARGE_INTEGER t1, t2, t3;
        QueryPerformanceCounter(&t1);

            
        long long pos = 0, sum = 0;
        /* use SIZE/16 to speedup tests */
        for (long long i = 0; i < (SIZE/SIZE_DIV); ++i)
        {
            _mm_prefetch(array + ((pos + 1879048224) & SIZE_MASK), _MM_HINT_T0);

            sum += 179 % (sum + array[pos] + 1);
            sum += 178 % (sum + array[pos] + 1);
            sum += 177 % (sum + array[pos] + 1);
            sum += 176 % (sum + array[pos] + 1);
            sum += 175 % (sum + array[pos] + 1);
            sum += 174 % (sum + array[pos] + 1);
            sum += 173 % (sum + array[pos] + 1);
            sum += 172 % (sum + array[pos] + 1);
            sum += 171 % (sum + array[pos] + 1);
            sum += 170 % (sum + array[pos] + 1);
            sum += 57 % (sum + array[pos] + 1);

            
            pos = (pos + 998244353LL) & SIZE_MASK;
        }
        
        QueryPerformanceCounter(&t2);
        QueryPerformanceFrequency(&t3);

        {
            printf("UserTime: %.2lf e/us  ", ((double)SIZE/SIZE_DIV) / ((t2.QuadPart - t1.QuadPart) / (double)t3.QuadPart * 1.0e6));
            printf("KernelTime: not measured e/us, sum = %lld\n", sum);
        }
    }
}

