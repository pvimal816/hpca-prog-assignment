#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include<sys/mman.h>
// Create other necessary functions here

__global__ void diag_mult_kernel(int *dev_matA, int *dev_matB, int *dev_output, int N){
    __shared__ int s[32][33];
    int i = blockIdx.y*blockDim.y + threadIdx.y;
    int j = blockIdx.x*blockDim.x + threadIdx.x;
    s[threadIdx.y][threadIdx.x] = dev_matB[i * N + j];
    __syncthreads();
    i = (N/blockDim.x-blockIdx.x-1)*blockDim.x;
    j = blockIdx.y*blockDim.y;
    atomicAdd(dev_output + i + threadIdx.y + j + threadIdx.x, dev_matA[(i+threadIdx.y) * N + j + threadIdx.x] * s[threadIdx.x][32-threadIdx.y-1]);
}

// Fill in this function
void gpuThread(int N, int *matA, int *matB, int *output)
{
    int *dev_matA = 0;
    int *dev_matB = 0;
    int *dev_output = 0;
    cudaError_t cudaStatus;
    
    dim3 threads_per_block(32, 32);
    dim3 no_of_blocks(N/threads_per_block.x, N/threads_per_block.y);

    cudaStatus = cudaSetDevice(0);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaSetDevice failed!  Do you have a CUDA-capable GPU installed?");
        goto Error;
    }

    cudaStatus = cudaMalloc((void**) &dev_matA, N*N*sizeof(int));
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMalloc failed!");
        goto Error;
    }

    cudaStatus = cudaMalloc((void**) &dev_matB, N*N*sizeof(int));
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMalloc failed!");
        goto Error;
    }
    
    cudaStatus = cudaMemcpy(dev_matA, matA, N*N*sizeof(int), cudaMemcpyHostToDevice);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMemcpy failed!");
        goto Error;
    }

    cudaStatus = cudaMemcpy(dev_matB, matB, N*N*sizeof(int), cudaMemcpyHostToDevice);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMemcpy failed!");
        goto Error;
    }

    cudaStatus = cudaMalloc((void**) &dev_output, (2*N-1)*sizeof(int));
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMalloc failed!");
        goto Error;
    }

    cudaStatus = cudaMemset(dev_output, 0, (2*N-1)*sizeof(int));
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMemset failed!");
        goto Error;
    }

    diag_mult_kernel<<<no_of_blocks, threads_per_block>>>(dev_matA, dev_matB, dev_output, N);

    // Check for any errors launching the kernel
    cudaStatus = cudaGetLastError();
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "diag_mult_kernel launch failed: %s\n", cudaGetErrorString(cudaStatus));
        goto Error;
    }
    
    cudaStatus = cudaMemcpy(output, dev_output, (2*N-1)*sizeof(int), cudaMemcpyDeviceToHost);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMemcpy failed!");
        goto Error;
    }

    Error:
    cudaFree(dev_output);
    cudaFree(dev_matA);
    cudaFree(dev_matB);
}
