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
        HANDLE hThr = GetCurrentThread();
        
        /* measure loop with fetching */
        FILETIME CreationTime, ExitTime, KernelTime, UserTime;
        GetThreadTimes(hThr, &CreationTime, &ExitTime, &KernelTime, &UserTime);

        
        long long pos = 0;
        /* use SIZE/16 to speedup tests */
        for (long long i = 0; i < (SIZE/16); ++i)
        {
            PREFETCH_POSITION;
            array[pos] = 179 % (i + 1);
            pos = (pos + 998244353LL) & SIZE_MASK;
        }
        
        FILETIME CreationTime2, ExitTime2, KernelTime2, UserTime2;
        GetThreadTimes(hThr, &CreationTime2, &ExitTime2, &KernelTime2, &UserTime2);

        {
            long long t1, t2;
            t1 = *(long long *)&UserTime;
            t2 = *(long long *)&UserTime2;
            printf("UserTime: %.2lf e/us  ", (SIZE/16.0) / ((t2 - t1) / 10.0));
            t1 = *(long long *)&KernelTime;
            t2 = *(long long *)&KernelTime2;
            printf("KernelTime: %.2lf e/us\n", (SIZE/16.0) / ((t2 - t1) / 10.0));
        }
    }
}
