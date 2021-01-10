#include <pthread.h>
#include <cstring>
#include <immintrin.h>
// #include <zmmintrin.h>

int n = 0;
int *a = NULL;
int *c = NULL;
int *op = NULL;
int NTHREADS = 8;
const int BS = 16; //cache block size

#ifdef __AVX512__
#define CS 16 //chunk size
#endif

#ifndef __AVX512__
#ifdef __AVX2__
#define CS 8 //chunk size
#endif
#endif

void *rotate_worker(void *arg)
{
    int id = *((int *)arg);
    int end = (id+1)*n/NTHREADS;
    //process data in 16x16 block to reduce LLC misses.
    for (int i = id*n/NTHREADS; i < end; i += BS)
    {
        for (int j = 0; j + BS - 1 < n; j += BS)
        {
            for (register int k1 = i; k1 < i + BS; k1++)
            {
                for (register int k2 = j; k2 < j + BS;)
                {
                    //unroll loop to reduce branch misses by a factor of 4
                    //overly unrolling leads to poor performance maybe because of misses in 
                    //microop cache(seen from intel performance optimization manual)
                    c[k1 * n + k2] = a[k2 * n + n - k1 - 1], k2+=1;
                    c[k1 * n + k2] = a[k2 * n + n - k1 - 1], k2 += 1;
                    c[k1 * n + k2] = a[k2 * n + n - k1 - 1], k2 += 1;
                    c[k1 * n + k2] = a[k2 * n + n - k1 - 1], k2 += 1;
                }
            }
        }
    }
}

int *multi_threaded_fast_rotate_270(int N, int *A)
{
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

void *multiply_worker(void *arg){
    //upper diagonals
    int diag_start = *(int *) arg;
    int diag_end = *((int *) arg + 1);
    
#ifdef __AVX512__
    for (int i = 0; i < diag_end; i++)
    {
        int n_chunk = ((diag_end - i) - max(diag_start - i, 0))/16;
        int j = max(diag_start - i, 0);
        for(; n_chunk>0; --n_chunk, j+=16){
            __m512i a_ = _mm512_loadu_si512((__m512i*) &a[i*n+j]);
            __m512i c_ = _mm512_loadu_si512((__m512i *)&c[i * n + j]);
            __m512i op_ = _mm512_loadu_si512((__m512i *)&op[i+j]);
            a_ = _mm512_mullo_epi32(a_, c_);
            op_ = _mm512_add_epi32(op_, a_);
            _mm512_storeu_si512((__m512i*)&op[i+j], op_);
        }
        for (; j < diag_end - i; j++)
        {
            op[i + j] += a[i * n + j] * c[i * n + j];
        }
    }
    //lower diagonals
    diag_start += n;
    diag_end = min(diag_end + n, 2 * n - 1);
    for (int i = diag_start - n; i < n; i++)
    {
        int j = diag_start - i;
        int n_chunk = (min(diag_end - i, n) - j)/16;
        for (; n_chunk > 0; --n_chunk, j += 16)
        {
            __m512i a_ = _mm512_loadu_si512((__m512i *)&a[i * n + j]);
            __m512i c_ = _mm512_loadu_si512((__m512i *)&c[i * n + j]);
            __m512i op_ = _mm512_loadu_si512((__m512i *)&op[i + j]);
            a_ = _mm512_mullo_epi32(a_, c_);
            op_ = _mm512_add_epi32(op_, a_);
            _mm512_storeu_si512((__m512i *)&op[i + j], op_);
        }
        for (; j < min(diag_end - i, n); j++)
        {
            op[i + j] += a[i * n + j] * c[i * n + j];
        }
    }
#else
#ifdef __AVX2__
    for (int i = 0; i < diag_end; i++)
    {
        int n_chunk = ((diag_end - i) - max(diag_start - i, 0)) / 8;
        int j = max(diag_start - i, 0);
        for (; n_chunk > 0; --n_chunk, j += 8)
        {
            __m256i a_ = _mm256_loadu_si256((__m256i *)&a[i * n + j]);
            __m256i c_ = _mm256_loadu_si256((__m256i *)&c[i * n + j]);
            __m256i op_ = _mm256_loadu_si256((__m256i *)&op[i + j]);
            a_ = _mm256_mullo_epi32(a_, c_);
            op_ = _mm256_add_epi32(op_, a_);
            _mm256_storeu_si256((__m256i *)&op[i + j], op_);
        }
        for (; j < diag_end - i; j++)
        {
            op[i + j] += a[i * n + j] * c[i * n + j];
        }
    }
    //lower diagonals
    diag_start += n;
    diag_end = min(diag_end + n, 2 * n - 1);
    for (int i = diag_start - n; i < n; i++)
    {
        int j = diag_start - i;
        int n_chunk = (j - max(diag_start - i, 0)) / 8;
        for (; n_chunk > 0; --n_chunk, j += 8)
        {
            __m256i a_ = _mm256_loadu_si256((__m256i *)&a[i * n + j]);
            __m256i c_ = _mm256_loadu_si256((__m256i *)&c[i * n + j]);
            __m256i op_ = _mm256_loadu_si256((__m256i *)&op[i + j]);
            a_ = _mm256_mullo_epi32(a_, c_);
            op_ = _mm256_add_epi32(op_, a_);
            _mm256_storeu_si256((__m256i *)&op[i + j], op_);
        }
        for (; j < min(diag_end - i, n); j++)
        {
            op[i + j] += a[i * n + j] * c[i * n + j];
        }
    }
#endif
#endif

#ifndef CS
    for(int i=0; i<diag_end; i++){
        for(int j=max(diag_start-i, 0); j<diag_end-i; j++){
            op[i+j] += a[i*n+j] * c[i*n+j];
        }
    }
    //lower diagonals
    diag_start += n;
    diag_end = min(diag_end+n, 2*n-1);
    for(int i=diag_start-n; i<n; i++){
        for(int j=diag_start-i; j<min(diag_end-i, n); j++){
            op[i + j] += a[i * n + j] * c[i * n + j];
        }
    }
#endif
}

// Fill in this function
void multiThread(int N, int *matA, int *matB, int *output, int thread_cnt)
{
    NTHREADS = thread_cnt;
    memset(output, 0, sizeof(int) * ((N << 1) - 1));
    n = N;
    op = output;
    c = multi_threaded_fast_rotate_270(N, matB);
    pthread_t threads[NTHREADS];                                                                                                                                                                                                                           
    int args[NTHREADS][2];
    int work_per_thread = N/NTHREADS;
    int ds = 0;
    for(int i=0; i<NTHREADS; i++){
        args[i][0] = ds, ds+=work_per_thread;
        args[i][1] = ds;
        pthread_create(&threads[i], NULL, multiply_worker, (void*)&args[i]);
    }
    for(int i=0; i<NTHREADS; i++){
        pthread_join(threads[i], NULL);
    }
    free(c);
}
