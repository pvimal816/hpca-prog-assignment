#include "cuda_runtime.h"
#include "device_launch_parameters.h"
// Create other necessary functions here

__global__ void diag_mult_kernel(int *dev_matA, int *dev_matB, int *dev_output, int N){
    int thread_id = blockIdx.x*blockDim.x + threadIdx.x;
    int i = thread_id;
    if(thread_id<N){
        int temp = 0;
        // Iterate over diagonal elements
        for(int j = 0; j < i + 1; ++j) {
            int rowA = j;
            int colA = i - j;
            int rowB = i - j;
            int colB = N - j - 1;
            temp += dev_matA[rowA * N + colA] * dev_matB[rowB * N + colB];
        }
        dev_output[i] = temp;
    }else if(thread_id<2*N-1){
        int temp = 0;
        // Iterate over diagonal elements
        for(int j = 0; j < 2 * N - (i + 1); ++j) {
            int rowA = i + 1 + j - N;
            int colA = N - j - 1;
            int rowB = N - j - 1;
            int colB = 2 * N - j - 2 - i;
            temp += dev_matA[rowA * N + colA] * dev_matB[rowB * N + colB];
        }
        dev_output[i] = temp;
    }
}

// Fill in this function
void gpuThread(int N, int *matA, int *matB, int *output)
{
    int *dev_matA = 0;
    int *dev_matB = 0;
    int *dev_output = 0;
    cudaError_t cudaStatus;
    
    int threads_per_blocks = 1024;
    int no_of_blocks = (2*N-1)/threads_per_blocks + ((2*N-1)%threads_per_blocks?1:0);

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

    cudaStatus = cudaMalloc((void**) &dev_output, (2*N-1)*sizeof(int));
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

    diag_mult_kernel<<<no_of_blocks, threads_per_blocks>>>(dev_matA, dev_matB, dev_output, N);

    // Check for any errors launching the kernel
    cudaStatus = cudaGetLastError();
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "addKernel launch failed: %s\n", cudaGetErrorString(cudaStatus));
        goto Error;
    }
    
    // cudaDeviceSynchronize waits for the kernel to finish, and returns
    // any errors encountered during the launch.
    cudaStatus = cudaDeviceSynchronize();
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaDeviceSynchronize returned error code %d after launching addKernel!\n", cudaStatus);
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
