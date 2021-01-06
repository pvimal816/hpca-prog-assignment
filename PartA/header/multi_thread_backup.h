#include <pthread.h>
#define NTHREADS 4
// Create other necessary functions here
int n=0;
int *A=NULL, *B=NULL, *op;

void *fun1(void* ptr){
    int i = *((int *) ptr);
    int temp = 0, temp1;
    for (int j=0; j<i+1; j++){
        int rowA = j;
        int colA = i-j;
        int rowB = i-j;
        int colB = n-j-1;
        temp1 = A[rowA*n+colA] * B[rowB*n+colB] + temp;
        temp = temp1;
    }
    op[i] = temp;
}

void *fun2(void *ptr)
{
    int i = *((int *)ptr);
    int temp = 0, temp1;
    // Iterate over diagonal elements
    for (int j = 0; j < 2 * n - (i + 1); ++j)
    {
        int rowA = i + 1 + j - n;
        int colA = n - j - 1;
        int rowB = n - j - 1;
        int colB = 2 * n - j - 2 - i;
        temp1 = A[rowA * n + colA] * B[rowB * n + colB] + temp;
        temp = temp1;
    }
    op[i] = temp;
}

// Fill in this function
void multiThread(int N, int *matA, int *matB, int *output)
{
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
}
