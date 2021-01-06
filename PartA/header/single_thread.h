#include <cstring>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <smmintrin.h>
#include <immintrin.h>

int *rotate_270(int n, int *A)
{   //This function is optimized to reduce DRAM accesses.
    //Still there seems some space to reduce L1 read misses.
    //Currently, L1 and DRAM read hitrate is 0% and 100% for array A.
    register int N = n;
    int *ret = (int *)aligned_alloc(128, N * N * sizeof(int));
    const int BS = 16; //cache block size
    for (int i = 0; i + BS - 1 < N; i += BS)
    {
        for (int j = 0; j + BS - 1 < N; j += BS)
        {   
            for (register int k1 = i; k1 < i + BS; k1++)
            {
                for (register int k2 = j; k2 < j + BS; k2++)
                {
                    ret[k1 * N + k2] = A[k2 * N + N - k1 - 1];
                }
            }
        }
    }
    return ret;
}

// Optimize this function
void singleThread(int N, int *matA, int *matB, int *output)
{
    memset(output, 0, sizeof(int)*((N<<1)-1));
    int* C = rotate_270(N, matB);
    for(int i = 0; i < N; ++i) {
        for(int j=0; j<N; j+=8) {
            __m256i a = _mm256_loadu_si256((__m256i *) (matA + (i * N + j)));
            __m256i b = _mm256_loadu_si256((__m256i *) (C + (i * N + j)));
            a = _mm256_mullo_epi32(a, b);
            __m256i op = _mm256_loadu_si256((__m256i *)(output + i + j));
            op = _mm256_add_epi32(op, a);
            _mm256_storeu_si256((__m256i*)(output+i+j), op);
        }
    }
}