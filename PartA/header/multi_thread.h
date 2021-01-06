#include <pthread.h>
#include <cstring>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <smmintrin.h>
#include <immintrin.h>

int n = 0;
int *a = NULL;
int *c = NULL;
const int NTHREADS = 4;
const int BS = 16; //cache block size

void *rotate_worker(void *arg)
{
    int id = *((int *)arg);
    for (int i = id*BS; i + BS - 1 < n; i += BS * NTHREADS)
    {
        for (int j = 0; j + BS - 1 < n; j += BS)
        {
            for (int k1 = i; k1 < i + BS; k1++)
            {
                for (int k2 = j; k2 < j + BS; k2++)
                {
                    c[k1 * n + k2] = a[k2 * n + n - k1 - 1];
                }
            }
        }
    }
}

int *multi_threaded_fast_rotate_270(int N, int *A)
{
    n = N;
    a = A;
    c = (int *)aligned_alloc(128, N * N * sizeof(int));
    int args[NTHREADS];
    pthread_t threads[NTHREADS];
    for(int i=0; i<NTHREADS; i++){
        args[i] = i;
        pthread_create(&threads[i], NULL, rotate_worker, (void *)&args[i]);
    }
    for (int i = 0; i < NTHREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }
    return c;
}

// Fill in this function
void multiThread(int N, int *matA, int *matB, int *output)
{

    memset(output, 0, sizeof(int) * ((N << 1) - 1));
    int *C = multi_threaded_fast_rotate_270(N, matB);
    for (int i = 0; i < N; ++i)
    {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                
        for (int j = 0; j < N; j += 8)
        {
            __m256i a = _mm256_loadu_si256((__m256i *)(matA + (i * N + j)));
            __m256i b = _mm256_loadu_si256((__m256i *)(C + (i * N + j)));
            a = _mm256_mullo_epi32(a, b);
            __m256i op = _mm256_loadu_si256((__m256i *)(output + i + j));
            op = _mm256_add_epi32(op, a);
            _mm256_storeu_si256((__m256i *)(output + i + j), op);
            // output[i + j] += matA[i * N + j] * matB[j * N + N - i - 1];
        }
    }                                                                                                                                                                                                                           
    free(C);
/*
    A = matA;
    B = matB;
    n = N;
    op = output;
    
    pthread_t threads[NTHREADS];
    int args[NTHREADS];
    int i;
    for(i=0; i+NTHREADS<N; i+=NTHREADS){
        for(int j=0; j<NTHREADS; j++){
            args[j] = i+j;
            pthread_create(&threads[j], NULL, fun1, (void*)&args[j]);
        }
        for(int j=0; j<NTHREADS; j++){
            pthread_join(threads[j], NULL);
        }
    }

    //handling leftover
    for (; i < N; i++)
    {
        fun1((void *)&i);
    }

    for (i = N; i+NTHREADS < 2*N-1; i += NTHREADS)
    {
        for (int j = 0; j < NTHREADS; j++)
        {
            args[j] = i + j;
            pthread_create(&threads[j], NULL, fun2, (void*)&args[j]);
        }
        for (int j = 0; j < NTHREADS; j++)
        {
            pthread_join(threads[j], NULL);
        }
    }

    //handling leftover
    for(; i<2*N-1; i++){
        fun2((void*)&i);
    }

*/
}
