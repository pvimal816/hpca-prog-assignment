#include <cstring>

void singleThread(int N, int *matA, int *matB, int *output)
{
    const int BS = 16; //cache block size
    for (int i = 0; i + BS - 1 < N; i += BS)
    {
        for (int j = 0; j + BS - 1 < N; j += BS)
        {
            for (register int k1 = i; k1 < i + BS; k1++)
            {
                for (register int k2 = j; k2 < j + BS;)
                {
                    output[k1 + k2] += matA[k1*N+k2] * matB[k2 * N + N - k1 - 1], ++k2;
                    output[k1 + k2] += matA[k1 * N + k2] * matB[k2 * N + N - k1 - 1], ++k2;
                    output[k1 + k2] += matA[k1 * N + k2] * matB[k2 * N + N - k1 - 1], ++k2;
                    output[k1 + k2] += matA[k1 * N + k2] * matB[k2 * N + N - k1 - 1], ++k2;
                }
            }
        }
    }
}
