#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>

// Cuda
#include <cuda.h>
#include <cuda_runtime.h>


#include "KeccakF.h"
#include "KeccakTreeCPU.h"
#include "KeccakTreeGPU.h"

#include "Test.h"

unsigned char seed;

static unsigned long long nano(struct timespec* t)
{
    return t->tv_sec * 1000000000ULL + t->tv_nsec;
}

static double speed1(struct timespec* t1, struct timespec* t2, unsigned imax)
{
    return INPUT_BLOCK_SIZE_B * NB_THREADS * NB_THREADS_BLOCKS *
           NB_INPUT_BLOCK * imax * 1e3 / (nano(t2) - nano(t1));
}

#define TOTAL_INPUT_SIZE \
    (INPUT_BLOCK_SIZE_B * NB_THREADS * NB_THREADS_BLOCKS * NB_INPUT_BLOCK)
#define TOTAL_OUTPUT_SIZE (OUTPUT_BLOCK_SIZE_B * NB_THREADS * NB_THREADS_BLOCKS)

void TestCPU(unsigned imax)
{
    struct timespec t1, t2;
    int i;

    unsigned* h_inBuffer;   // Host in buffer for data to be hashed
    unsigned* h_outBuffer;  // Host out buffer

    unsigned Kstate[25];  // Keccak State for top node
    memset(Kstate, 0, 25 * sizeof(unsigned));


    // init host inBuffer
    h_inBuffer = (unsigned*)malloc(TOTAL_INPUT_SIZE);
    memset(h_inBuffer, 0, TOTAL_INPUT_SIZE);

    // init host outBuffer
    h_outBuffer = (unsigned*)malloc(TOTAL_OUTPUT_SIZE);
    memset(h_outBuffer, 0, TOTAL_OUTPUT_SIZE);

    //***************************
    // init h_inBuffer with values
    for (i = 0; i < INPUT_BLOCK_SIZE_B / 4 * NB_INPUT_BLOCK * NB_THREADS *
                        NB_THREADS_BLOCKS;
         i++)
    {
        h_inBuffer[i] = i + seed;
    }


    // CPU computation *******************************
    printf("CPU speed test started\n");

    clock_gettime(CLOCK_REALTIME, &t1);
    for (i = 0; i < imax; i++)
    {
        KeccakTreeCPU(h_inBuffer, h_outBuffer);
        Keccak_top(Kstate, h_outBuffer, NB_THREADS * NB_THREADS_BLOCKS);
    }
    clock_gettime(CLOCK_REALTIME, &t2);

    print_KS_256(Kstate);

    printf("CPU speed : %.2f mB/s \n\n", speed1(&t1, &t2, imax));


    // free all buffer host and device
    free(h_inBuffer);
    free(h_outBuffer);
}

void TestGPU(unsigned imax)
{
    struct timespec t1, t2;
    unsigned int i;

    unsigned* h_inBuffer;   // Host in buffer for data to be hashed
    unsigned* h_outBuffer;  // Host out buffer

    unsigned* d_inBuffer;   // device in buffer
    unsigned* d_outBuffer;  // device out buffer

    unsigned Kstate[25];  // Keccak State for top node
    memset(Kstate, 0, 25 * sizeof(unsigned));


    // init host inBuffer
    h_inBuffer = (unsigned*)malloc(TOTAL_INPUT_SIZE);
    memset(h_inBuffer, 0, TOTAL_INPUT_SIZE);

    // init host outBuffer
    h_outBuffer = (unsigned*)malloc(TOTAL_OUTPUT_SIZE);
    memset(h_outBuffer, 0, TOTAL_OUTPUT_SIZE);

    // init device inBuffer
    cudaMalloc((void**)&d_inBuffer, TOTAL_INPUT_SIZE);
    checkCUDAError(" cudaMalloc d_inBuffer");
    cudaMemset(d_inBuffer, 0, TOTAL_INPUT_SIZE);
    checkCUDAError(" cudaMemset d_inBuffer");

    // init device outBuffer
    cudaMalloc((void**)&d_outBuffer, TOTAL_OUTPUT_SIZE);
    checkCUDAError(" cudaMalloc d_outBuffer");
    cudaMemset(d_outBuffer, 0, TOTAL_OUTPUT_SIZE);
    checkCUDAError(" cudaMemset d_outBuffer");

    //***************************
    // init h_inBuffer with values
    for (i = 0; i < INPUT_BLOCK_SIZE_B / 4 * NB_INPUT_BLOCK * NB_THREADS *
                        NB_THREADS_BLOCKS;
         i++)
    {
        h_inBuffer[i] = i + seed;
    }

    // GPU computation *******************************
    printf("GPU speed test started\n");

    clock_gettime(CLOCK_REALTIME, &t1);
    for (i = 0; i < imax; i++)
    {
        KeccakTreeGPU(h_inBuffer, d_inBuffer, h_outBuffer, d_outBuffer);
        Keccak_top(Kstate, h_outBuffer, NB_THREADS * NB_THREADS_BLOCKS);
    }

    clock_gettime(CLOCK_REALTIME, &t2);

    print_KS_256(Kstate);

    printf("GPU speed : %.2f mB/s \n\n", speed1(&t1, &t2, imax));


    // free all buffer host and device
    free(h_inBuffer);
    free(h_outBuffer);

    cudaFree(d_inBuffer);
    cudaFree(d_outBuffer);
}


void TestGPU_OverlapCPU(unsigned imax)
{
    struct timespec t1, t2;
    int i;

    unsigned* h_inBuffer;   // Host in buffer for data to be hashed
    unsigned* h_outBuffer;  // Host out buffer

    unsigned* d_inBuffer;   // device in buffer
    unsigned* d_outBuffer;  // device out buffer

    unsigned Kstate[25];  // Keccak State for top node
    memset(Kstate, 0, 25 * sizeof(unsigned));


    // init host inBuffer
    h_inBuffer = (unsigned*)malloc(TOTAL_INPUT_SIZE);
    memset(h_inBuffer, 0, TOTAL_INPUT_SIZE);

    // init host outBuffer
    h_outBuffer = (unsigned*)malloc(TOTAL_OUTPUT_SIZE);
    memset(h_outBuffer, 0, TOTAL_OUTPUT_SIZE);

    // init device inBuffer
    cudaMalloc((void**)&d_inBuffer, TOTAL_INPUT_SIZE);
    checkCUDAError(" cudaMalloc d_inBuffer");
    cudaMemset(d_inBuffer, 0, TOTAL_INPUT_SIZE);
    checkCUDAError(" cudaMemset d_inBuffer");

    // init device outBuffer
    cudaMalloc((void**)&d_outBuffer, TOTAL_OUTPUT_SIZE);
    checkCUDAError(" cudaMalloc d_outBuffer");
    cudaMemset(d_outBuffer, 0, TOTAL_OUTPUT_SIZE);
    checkCUDAError(" cudaMemset d_outBuffer");

    //***************************
    // init h_inBuffer with values
    for (i = 0; i < INPUT_BLOCK_SIZE_B / 4 * NB_INPUT_BLOCK * NB_THREADS *
                        NB_THREADS_BLOCKS;
         i++)
    {
        h_inBuffer[i] = i + seed;
    }


    // GPU computation *******************************

    printf("GPU speed overlap CPU test started\n");

    clock_gettime(CLOCK_REALTIME, &t1);

    // first iteration
    KeccakTreeGPU(h_inBuffer, d_inBuffer, h_outBuffer, d_outBuffer);

    // other iteration overlapping GPU computation with CPU
    for (i = 1; i < imax; i++)
    {
        KeccakTreeGPU_overlapCPU(
            h_inBuffer, d_inBuffer, h_outBuffer, d_outBuffer, Kstate);
    }
    // last output block on CPU
    Keccak_top(Kstate, h_outBuffer, NB_THREADS * NB_THREADS_BLOCKS);

    clock_gettime(CLOCK_REALTIME, &t2);
    print_KS_256(Kstate);

    printf("GPU speed overlap CPU : %.2f mB/s \n\n", speed1(&t1, &t2, imax));

    // free all buffer host and device
    free(h_inBuffer);
    free(h_outBuffer);

    cudaFree(d_inBuffer);
    cudaFree(d_outBuffer);
}


void TestGPU_Split(unsigned imax)
{
    struct timespec t1, t2;
    int i;

    unsigned* h_inBuffer;   // Host in buffer for data to be hashed
    unsigned* h_outBuffer;  // Host out buffer

    unsigned* d_inBuffer;   // device in buffer
    unsigned* d_outBuffer;  // device out buffer

    unsigned Kstate[25];  // Keccak State for top node
    memset(Kstate, 0, 25 * sizeof(unsigned));


    // init host inBuffer
    h_inBuffer = (unsigned*)malloc(TOTAL_INPUT_SIZE);
    memset(h_inBuffer, 0, TOTAL_INPUT_SIZE);

    // init host outBuffer
    h_outBuffer = (unsigned*)malloc(TOTAL_OUTPUT_SIZE);
    memset(h_outBuffer, 0, TOTAL_OUTPUT_SIZE);

    // init device inBuffer
    cudaMalloc((void**)&d_inBuffer, TOTAL_INPUT_SIZE);
    checkCUDAError(" cudaMalloc d_inBuffer");
    cudaMemset(d_inBuffer, 0, TOTAL_INPUT_SIZE);
    checkCUDAError(" cudaMemset d_inBuffer");

    // init device outBuffer
    cudaMalloc((void**)&d_outBuffer, TOTAL_OUTPUT_SIZE);
    checkCUDAError(" cudaMalloc d_outBuffer");
    cudaMemset(d_outBuffer, 0, TOTAL_OUTPUT_SIZE);
    checkCUDAError(" cudaMemset d_outBuffer");

    //***************************
    // init h_inBuffer with values
    for (i = 0; i < INPUT_BLOCK_SIZE_B / 4 * NB_INPUT_BLOCK * NB_THREADS *
                        NB_THREADS_BLOCKS;
         i++)
    {
        h_inBuffer[i] = i + seed;
    }

    // GPU computation *******************************

    clock_gettime(CLOCK_REALTIME, &t1);
    for (i = 0; i < imax; i++)
    {
        KeccakTreeGPU_Split(h_inBuffer, d_inBuffer, h_outBuffer, d_outBuffer);
        Keccak_top(Kstate, h_outBuffer, NB_THREADS * NB_THREADS_BLOCKS);
    }

    clock_gettime(CLOCK_REALTIME, &t2);

    print_KS_256(Kstate);

    printf("GPU Split speed : %.2f mB/s \n\n", speed1(&t1, &t2, imax));


    // free all buffer host and device
    free(h_inBuffer);
    free(h_outBuffer);

    cudaFree(d_inBuffer);
    cudaFree(d_outBuffer);
}

void TestGPU_Stream(unsigned imax)
{
    struct timespec t1, t2;
    int i;

    unsigned* h_inBuffer = NULL;   // Host in buffer for data to be hashed
    unsigned* h_outBuffer = NULL;  // Host out buffer

    unsigned* d_inBuffer;   // device in buffer
    unsigned* d_outBuffer;  // device out buffer

    unsigned Kstate[25];  // Keccak State for top node
    memset(Kstate, 0, 25 * sizeof(unsigned));


    // alloc host inBuffer WITH CudaHostAlloc
    cudaHostAlloc((void**)&h_inBuffer, TOTAL_INPUT_SIZE, cudaHostAllocDefault);
    checkCUDAError("cudaMallocHost h_inBuffer");

    memset(h_inBuffer, 0, TOTAL_INPUT_SIZE);

    // alloc host outBuffer WITH CudaHostAlloc
    cudaHostAlloc(
        (void**)&h_outBuffer, TOTAL_OUTPUT_SIZE, cudaHostAllocDefault);
    checkCUDAError("cudaMallocHost h_outBuffer");

    memset(h_outBuffer, 0, TOTAL_OUTPUT_SIZE);

    // init device inBuffer
    cudaMalloc((void**)&d_inBuffer, TOTAL_INPUT_SIZE);
    checkCUDAError(" cudaMalloc d_inBuffer");
    cudaMemset(d_inBuffer, 0, TOTAL_INPUT_SIZE);
    checkCUDAError(" cudaMemset d_inBuffer");

    // init device outBuffer
    cudaMalloc((void**)&d_outBuffer, TOTAL_OUTPUT_SIZE);
    checkCUDAError(" cudaMalloc d_outBuffer");
    cudaMemset(d_outBuffer, 0, TOTAL_OUTPUT_SIZE);
    checkCUDAError(" cudaMemset d_outBuffer");

    //***************************
    // init h_inBuffer with values
    for (i = 0; i < INPUT_BLOCK_SIZE_B / 4 * NB_INPUT_BLOCK * NB_THREADS *
                        NB_THREADS_BLOCKS;
         i++)
    {
        h_inBuffer[i] = i + seed;
    }

    // GPU computation *******************************
    printf("GPU Stream speed test started\n");

    clock_gettime(CLOCK_REALTIME, &t1);
    for (i = 0; i < imax; i++)
    {
        KeccakTreeGPU_Stream(h_inBuffer, d_inBuffer, h_outBuffer, d_outBuffer);
        Keccak_top(Kstate, h_outBuffer, NB_THREADS * NB_THREADS_BLOCKS);
    }

    clock_gettime(CLOCK_REALTIME, &t2);

    print_KS_256(Kstate);

    printf("GPU Stream speed : %.2f mB/s \n\n", speed1(&t1, &t2, imax));


    // free all buffer host and device
    cudaFreeHost(h_inBuffer);
    cudaFreeHost(h_outBuffer);

    cudaFree(d_inBuffer);
    cudaFree(d_outBuffer);
}


void TestGPU_Stream_OverlapCPU(unsigned imax)
{
    struct timespec t1, t2;
    int i;

    unsigned* h_inBuffer = NULL;   // Host in buffer for data to be hashed
    unsigned* h_outBuffer = NULL;  // Host out buffer

    unsigned* d_inBuffer;   // device in buffer
    unsigned* d_outBuffer;  // device out buffer

    unsigned Kstate[25];  // Keccak State for top node
    memset(Kstate, 0, 25 * sizeof(unsigned));


    // alloc host inBuffer WITH CudaHostAlloc
    cudaHostAlloc((void**)&h_inBuffer, TOTAL_INPUT_SIZE, cudaHostAllocDefault);
    checkCUDAError("cudaMallocHost h_inBuffer");

    memset(h_inBuffer, 0, TOTAL_INPUT_SIZE);

    // alloc host outBuffer WITH CudaHostAlloc
    cudaHostAlloc(
        (void**)&h_outBuffer, TOTAL_OUTPUT_SIZE, cudaHostAllocDefault);
    checkCUDAError("cudaMallocHost h_outBuffer");

    memset(h_outBuffer, 0, TOTAL_OUTPUT_SIZE);

    // init device inBuffer
    cudaMalloc((void**)&d_inBuffer, TOTAL_INPUT_SIZE);
    checkCUDAError(" cudaMalloc d_inBuffer");
    cudaMemset(d_inBuffer, 0, TOTAL_INPUT_SIZE);
    checkCUDAError(" cudaMemset d_inBuffer");

    // init device outBuffer
    cudaMalloc((void**)&d_outBuffer, TOTAL_OUTPUT_SIZE);
    checkCUDAError(" cudaMalloc d_outBuffer");
    cudaMemset(d_outBuffer, 0, TOTAL_OUTPUT_SIZE);
    checkCUDAError(" cudaMemset d_outBuffer");

    //***************************
    // init h_inBuffer with values
    for (i = 0; i < INPUT_BLOCK_SIZE_B / 4 * NB_INPUT_BLOCK * NB_THREADS *
                        NB_THREADS_BLOCKS;
         i++)
    {
        h_inBuffer[i] = i + seed;
    }

    // GPU computation *******************************
    printf("GPU Stream OverlapCPU speed test started\n");

    clock_gettime(CLOCK_REALTIME, &t1);

    // first iteration WITH STREAMS
    KeccakTreeGPU_Stream(h_inBuffer, d_inBuffer, h_outBuffer, d_outBuffer);

    for (i = 1; i < imax; i++)
    {
        KeccakTreeGPU_Stream_OverlapCPU(
            h_inBuffer, d_inBuffer, h_outBuffer, d_outBuffer, Kstate);
    }
    // last CPU computation
    Keccak_top(Kstate, h_outBuffer, NB_THREADS * NB_THREADS_BLOCKS);

    clock_gettime(CLOCK_REALTIME, &t2);

    print_KS_256(Kstate);

    printf(
        "GPU Stream OverlapCPU speed : %.2f mB/s \n\n", speed1(&t1, &t2, imax));


    // free all buffer host and device
    cudaFreeHost(h_inBuffer);
    cudaFreeHost(h_outBuffer);

    cudaFree(d_inBuffer);
    cudaFree(d_outBuffer);
}


// Using Mapped memory
// UNTESTED not supported by Author's GPU Hardware
void TestGPU_MappedMemory(unsigned imax)
{
    struct timespec t1, t2;
    int i;


    unsigned* h_inBuffer;   // Host in buffer for data to be hashed
    unsigned* h_outBuffer;  // Host out buffer

    unsigned* d_inBuffer;   // device in buffer pointer
    unsigned* d_outBuffer;  // device out buffer pointer

    // check device Prop
    struct cudaDeviceProp deviceProp;

    unsigned Kstate[25];  // Keccak State for top node
    memset(Kstate, 0, 25 * sizeof(unsigned));


    // Get properties and verify device 0 supports mapped memory
    cudaGetDeviceProperties(&deviceProp, 0);
    checkCUDAError("cudaGetDeviceProperties");


    if (!deviceProp.canMapHostMemory)
    {
        fprintf(stderr, "Device %d cannot map host memory!\n", 0);
        exit(EXIT_FAILURE);
    }


    // init host inBuffer WITH CudaHostAlloc
    cudaHostAlloc((void**)&h_inBuffer, TOTAL_INPUT_SIZE, cudaHostAllocMapped);
    checkCUDAError("cudaHostAlloc Mapped inBuffer");

    memset(h_inBuffer, 0, TOTAL_INPUT_SIZE);


    // init host outBuffer WITH CudaHostAlloc
    cudaHostAlloc((void**)&h_outBuffer, TOTAL_OUTPUT_SIZE, cudaHostAllocMapped);
    checkCUDAError("cudaHostAlloc Mapped outBuffer");

    memset(h_outBuffer, 0, TOTAL_OUTPUT_SIZE);

    // retreive device pointer to inBuffer mapped memory
    cudaHostGetDevicePointer((void**)&d_inBuffer, (void*)h_inBuffer, 0);
    checkCUDAError("cudaHostGetDevicePointer d_inBuffer");

    // retreive device pointer to outBuffer mapped memory
    cudaHostGetDevicePointer((void**)&d_outBuffer, (void*)h_outBuffer, 0);
    checkCUDAError("cudaHostGetDevicePointer d_outBuffer");


    // GPU computation *******************************

    clock_gettime(CLOCK_REALTIME, &t1);
    for (i = 0; i < imax; i++)
    {
        KeccakTreeGPU(h_inBuffer, d_inBuffer, h_outBuffer, d_outBuffer);
        Keccak_top(Kstate, h_outBuffer, NB_THREADS * NB_THREADS_BLOCKS);
    }

    clock_gettime(CLOCK_REALTIME, &t2);
    print_KS_256(Kstate);

    printf("GPU speed Mapped : %.2f mB/s \n", speed1(&t1, &t2, imax));


    // free all host buffers and device pointers
    cudaFreeHost(h_inBuffer);
    cudaFreeHost(h_outBuffer);
}

//**************************************************
// Use 2 stages hashtree
//**************************************************
void TestCPU_2stg(unsigned imax)
{
    struct timespec t1, t2;
    int i;

    unsigned* h_inBuffer;   // Host in buffer for data to be hashed
    unsigned* h_outBuffer;  // Host out buffer

    unsigned Kstate[25];  // Keccak State for top node
    memset(Kstate, 0, 25 * sizeof(unsigned));


    // init host inBuffer
    h_inBuffer = (unsigned*)malloc(TOTAL_INPUT_SIZE);
    memset(h_inBuffer, 0, TOTAL_INPUT_SIZE);

    // init host outBuffer size :  2*OUTPUT_BLOCK_SIZE_B * NB_SCND_STAGE_THREADS
    // * NB_THREADS_BLOCKS
    h_outBuffer = (unsigned*)malloc(
        2 * OUTPUT_BLOCK_SIZE_B * NB_SCND_STAGE_THREADS * NB_THREADS_BLOCKS);
    memset(h_outBuffer, 0,
        2 * OUTPUT_BLOCK_SIZE_B * NB_SCND_STAGE_THREADS * NB_THREADS_BLOCKS);

    //***************************
    // init h_inBuffer with values
    for (i = 0; i < INPUT_BLOCK_SIZE_B / 4 * NB_INPUT_BLOCK * NB_THREADS *
                        NB_THREADS_BLOCKS;
         i++)
    {
        h_inBuffer[i] = i + seed;
    }


    // CPU computation *******************************
    printf("CPU_2stg speed test started\n");

    clock_gettime(CLOCK_REALTIME, &t1);
    for (i = 0; i < (imax); i++)
    {
        KeccakTreeCPU_2stg(h_inBuffer, h_outBuffer);
        Keccak_top(
            Kstate, h_outBuffer, 2 * NB_SCND_STAGE_THREADS * NB_THREADS_BLOCKS);
    }
    clock_gettime(CLOCK_REALTIME, &t2);

    print_KS_256(Kstate);

    printf("CPU_2stg speed : %.2f mB/s \n\n", speed1(&t1, &t2, imax));


    // free all buffer host and device
    free(h_inBuffer);
    free(h_outBuffer);
}


// GPU 2 stages
void TestGPU_2stg(unsigned imax)
{
    struct timespec t1, t2;
    unsigned int i;

    unsigned* h_inBuffer;   // Host in buffer for data to be hashed
    unsigned* h_outBuffer;  // Host out buffer

    unsigned* d_inBuffer;   // device in buffer
    unsigned* d_outBuffer;  // device out buffer

    unsigned Kstate[25];  // Keccak State for top node
    memset(Kstate, 0, 25 * sizeof(unsigned));


    // init host inBuffer
    h_inBuffer = (unsigned*)malloc(TOTAL_INPUT_SIZE);
    memset(h_inBuffer, 0, TOTAL_INPUT_SIZE);

    // init host outBuffer
    h_outBuffer = (unsigned*)malloc(
        2 * OUTPUT_BLOCK_SIZE_B * NB_SCND_STAGE_THREADS * NB_THREADS_BLOCKS);
    memset(h_outBuffer, 0,
        2 * OUTPUT_BLOCK_SIZE_B * NB_SCND_STAGE_THREADS * NB_THREADS_BLOCKS);

    // init device inBuffer
    cudaMalloc((void**)&d_inBuffer, TOTAL_INPUT_SIZE);
    checkCUDAError(" cudaMalloc d_inBuffer");
    cudaMemset(d_inBuffer, 0, TOTAL_INPUT_SIZE);
    checkCUDAError(" cudaMemset d_inBuffer");

    // init device outBuffer
    cudaMalloc((void**)&d_outBuffer,
        2 * OUTPUT_BLOCK_SIZE_B * NB_SCND_STAGE_THREADS * NB_THREADS_BLOCKS);
    checkCUDAError(" cudaMalloc d_outBuffer");
    cudaMemset(d_outBuffer, 0,
        2 * OUTPUT_BLOCK_SIZE_B * NB_SCND_STAGE_THREADS * NB_THREADS_BLOCKS);
    checkCUDAError(" cudaMemset d_outBuffer");

    //***************************
    // init h_inBuffer with values
    for (i = 0; i < INPUT_BLOCK_SIZE_B / 4 * NB_INPUT_BLOCK * NB_THREADS *
                        NB_THREADS_BLOCKS;
         i++)
    {
        h_inBuffer[i] = i + seed;
    }

    // GPU computation *******************************
    printf("GPU_2stg speed test started\n");

    clock_gettime(CLOCK_REALTIME, &t1);
    for (i = 0; i < imax; i++)
    {
        KeccakTreeGPU_2stg(h_inBuffer, d_inBuffer, h_outBuffer, d_outBuffer);
        Keccak_top(
            Kstate, h_outBuffer, 2 * NB_SCND_STAGE_THREADS * NB_THREADS_BLOCKS);
    }

    clock_gettime(CLOCK_REALTIME, &t2);

    print_KS_256(Kstate);

    printf("GPU_2stg speed : %.2f mB/s \n\n", speed1(&t1, &t2, imax));


    // free all buffer host and device
    free(h_inBuffer);
    free(h_outBuffer);

    cudaFree(d_inBuffer);
    cudaFree(d_outBuffer);
}


void TestGPU_2stg_Stream_OverlapCPU(unsigned imax)
{
    struct timespec t1, t2;
    int i;

    unsigned* h_inBuffer = NULL;   // Host in buffer for data to be hashed
    unsigned* h_outBuffer = NULL;  // Host out buffer

    unsigned* d_inBuffer;   // device in buffer
    unsigned* d_outBuffer;  // device out buffer

    unsigned Kstate[25];  // Keccak State for top node
    memset(Kstate, 0, 25 * sizeof(unsigned));


    // alloc host inBuffer WITH CudaHostAlloc
    cudaHostAlloc((void**)&h_inBuffer, TOTAL_INPUT_SIZE, cudaHostAllocDefault);
    checkCUDAError("cudaMallocHost h_inBuffer");

    memset(h_inBuffer, 0, TOTAL_INPUT_SIZE);

    // alloc host outBuffer WITH CudaHostAlloc
    cudaHostAlloc((void**)&h_outBuffer,
        2 * OUTPUT_BLOCK_SIZE_B * NB_SCND_STAGE_THREADS * NB_THREADS_BLOCKS,
        cudaHostAllocDefault);
    checkCUDAError("cudaMallocHost h_outBuffer");

    memset(h_outBuffer, 0,
        2 * OUTPUT_BLOCK_SIZE_B * NB_SCND_STAGE_THREADS * NB_THREADS_BLOCKS);

    // init device inBuffer
    cudaMalloc((void**)&d_inBuffer, TOTAL_INPUT_SIZE);
    checkCUDAError(" cudaMalloc d_inBuffer");
    cudaMemset(d_inBuffer, 0, TOTAL_INPUT_SIZE);
    checkCUDAError(" cudaMemset d_inBuffer");

    // init device outBuffer
    cudaMalloc((void**)&d_outBuffer,
        2 * OUTPUT_BLOCK_SIZE_B * NB_SCND_STAGE_THREADS * NB_THREADS_BLOCKS);
    checkCUDAError(" cudaMalloc d_outBuffer");
    cudaMemset(d_outBuffer, 0,
        2 * OUTPUT_BLOCK_SIZE_B * NB_SCND_STAGE_THREADS * NB_THREADS_BLOCKS);
    checkCUDAError(" cudaMemset d_outBuffer");

    //***************************
    // init h_inBuffer with values
    for (i = 0; i < INPUT_BLOCK_SIZE_B / 4 * NB_INPUT_BLOCK * NB_THREADS *
                        NB_THREADS_BLOCKS;
         i++)
    {
        h_inBuffer[i] = i + seed;
    }

    // GPU computation *******************************
    printf("GPU_2stg Stream OverlapCPU speed test started\n");

    clock_gettime(CLOCK_REALTIME, &t1);

    // first iteration
    KeccakTreeGPU_2stg(h_inBuffer, d_inBuffer, h_outBuffer, d_outBuffer);

    for (i = 1; i < imax; i++)
    {
        KeccakTreeGPU_2stg_Stream_OverlapCPU(
            h_inBuffer, d_inBuffer, h_outBuffer, d_outBuffer, Kstate);
    }
    // last CPU computation
    Keccak_top(
        Kstate, h_outBuffer, 2 * NB_SCND_STAGE_THREADS * NB_THREADS_BLOCKS);

    clock_gettime(CLOCK_REALTIME, &t2);

    print_KS_256(Kstate);

    printf("GPU_2stg Stream OverlapCPU speed : %.2f mB/s \n\n",
        speed1(&t1, &t2, imax));


    // free all buffer host and device
    cudaFreeHost(h_inBuffer);
    cudaFreeHost(h_outBuffer);

    cudaFree(d_inBuffer);
    cudaFree(d_outBuffer);
}


//***************************************
// Keccak Stream CipherMode
// Use 256 bit key and 256 bit Nonce
//***************************************
void TestGPU_SCipher(unsigned imax)
{
    struct timespec t1, t2;
    int i;

    unsigned* h_inKeyNonce = NULL;  // Host in buffer for Key and Nonce
    unsigned* h_outBuffer = NULL;   // Host out buffer

    unsigned* d_inKeyNonce;  // device in buffer
    unsigned* d_outBuffer;   // device out buffer


    // alloc host inBuffer WITH CudaHostAlloc
    cudaHostAlloc((void**)&h_inKeyNonce, 2 * 32, cudaHostAllocDefault);
    checkCUDAError("cudaMallocHost h_inKeyNonce");

    memset(h_inKeyNonce, 0, 2 * 32);

    // alloc host outBuffer WITH CudaHostAlloc
    cudaHostAlloc((void**)&h_outBuffer,
        SC_NB_OUTPUT_BLOCK * OUTPUT_BLOCK_SIZE_B * NB_THREADS *
            NB_THREADS_BLOCKS,
        cudaHostAllocDefault);
    checkCUDAError("cudaMallocHost h_outBuffer");

    memset(h_outBuffer, 0,
        SC_NB_OUTPUT_BLOCK * OUTPUT_BLOCK_SIZE_B * NB_THREADS *
            NB_THREADS_BLOCKS);

    // init device inKeyNonce
    cudaMalloc((void**)&d_inKeyNonce, 2 * 32);
    checkCUDAError(" cudaMalloc d_inKeyNonce");
    cudaMemset(d_inKeyNonce, 0, 2 * 32);
    checkCUDAError(" cudaMemset d_inKeyNonce");

    // init device outBuffer
    cudaMalloc((void**)&d_outBuffer, SC_NB_OUTPUT_BLOCK * OUTPUT_BLOCK_SIZE_B *
                                         NB_THREADS * NB_THREADS_BLOCKS);
    checkCUDAError(" cudaMalloc d_outBuffer");
    cudaMemset(d_outBuffer, 0,
        SC_NB_OUTPUT_BLOCK * OUTPUT_BLOCK_SIZE_B * NB_THREADS *
            NB_THREADS_BLOCKS);
    checkCUDAError(" cudaMemset d_outBuffer");

    // GPU computation *******************************
    printf("GPU SCipher speed test started\n");

    clock_gettime(CLOCK_REALTIME, &t1);
    for (i = 0; i < imax; i++)
    {
        KeccakSCipherGPU_Stream(
            h_inKeyNonce, d_inKeyNonce, h_outBuffer, d_outBuffer);
    }

    clock_gettime(CLOCK_REALTIME, &t2);

    unsigned nb_threads =
        NB_THREADS * SC_NB_OUTPUT_BLOCK * OUTPUT_BLOCK_SIZE_B / 4;
    printf("%08x ", h_outBuffer[0]);
    printf("%08x ", h_outBuffer[1]);
    printf("%08x ", h_outBuffer[nb_threads]);
    printf("%08x ", h_outBuffer[nb_threads + 1]);
    printf("\n\n");

    printf("GPU SCipher speed : %.2f mB/s \n\n", speed1(&t1, &t2, imax));

    // free all buffer host and device
    cudaFreeHost(h_inKeyNonce);
    cudaFreeHost(h_outBuffer);

    cudaFree(d_inKeyNonce);
    cudaFree(d_outBuffer);
}


//*******************
// Test completness (use of each word of input buffer)
//*******************
void Test_Completness()
{
    int i, res, ctr;

    unsigned* h_inBuffer = NULL;   // Host in buffer for data to be hashed
    unsigned* h_outBuffer = NULL;  // Host out buffer

    unsigned* d_inBuffer;   // device in buffer
    unsigned* d_outBuffer;  // device out buffer

    unsigned Ks1[25];  // Keccak State for top node
    unsigned Ks2[25];  // Keccak State for top node
    memset(Ks1, 0, 25 * sizeof(unsigned));
    memset(Ks2, 0, 25 * sizeof(unsigned));


    // alloc host inBuffer WITH CudaHostAlloc
    cudaHostAlloc((void**)&h_inBuffer, TOTAL_INPUT_SIZE, cudaHostAllocDefault);
    checkCUDAError("cudaMallocHost h_inBuffer");

    memset(h_inBuffer, 0, TOTAL_INPUT_SIZE);

    // alloc host outBuffer WITH CudaHostAlloc
    cudaHostAlloc(
        (void**)&h_outBuffer, TOTAL_OUTPUT_SIZE, cudaHostAllocDefault);
    checkCUDAError("cudaMallocHost h_outBuffer");

    memset(h_outBuffer, 0, TOTAL_OUTPUT_SIZE);

    // init device inBuffer
    cudaMalloc((void**)&d_inBuffer, TOTAL_INPUT_SIZE);
    checkCUDAError(" cudaMalloc d_inBuffer");
    cudaMemset(d_inBuffer, 0, TOTAL_INPUT_SIZE);
    checkCUDAError(" cudaMemset d_inBuffer");

    // init device outBuffer
    cudaMalloc((void**)&d_outBuffer, TOTAL_OUTPUT_SIZE);
    checkCUDAError(" cudaMalloc d_outBuffer");
    cudaMemset(d_outBuffer, 0, TOTAL_OUTPUT_SIZE);
    checkCUDAError(" cudaMemset d_outBuffer");

    //***************************
    // init h_inBuffer with values
    for (i = 0; i < INPUT_BLOCK_SIZE_B / 4 * NB_INPUT_BLOCK * NB_THREADS *
                        NB_THREADS_BLOCKS;
         i++)
    {
        h_inBuffer[i] = i;
    }

    //***************************
    // compute the reference Hash in Ks1

    KeccakTreeGPU_Stream(h_inBuffer, d_inBuffer, h_outBuffer, d_outBuffer);
    Keccak_top(Ks1, h_outBuffer, NB_THREADS * NB_THREADS_BLOCKS);

    res = 0;
    ctr = 0;
    printf("Test Completness start \n");
    // Apply a change in all input words to see if hash is different
    for (i = 0; i < INPUT_BLOCK_SIZE_B / 4 * NB_INPUT_BLOCK * NB_THREADS *
                        NB_THREADS_BLOCKS;
         i++)
    {
        // zeroize device buffers and host result buffer
        cudaMemset(d_inBuffer, 0,
            INPUT_BLOCK_SIZE_B * NB_THREADS * NB_THREADS_BLOCKS *
                NB_INPUT_BLOCK);
        cudaMemset(d_outBuffer, 0, TOTAL_OUTPUT_SIZE);
        memset(h_outBuffer, 0, TOTAL_OUTPUT_SIZE);

        zeroize(Ks2);

        // change one word
        h_inBuffer[i] = h_inBuffer[i] + 3;

        KeccakTreeGPU_Stream(h_inBuffer, d_inBuffer, h_outBuffer, d_outBuffer);
        Keccak_top(Ks2, h_outBuffer, NB_THREADS * NB_THREADS_BLOCKS);

        if (isEqual_KS(Ks1, Ks2))
        {
            printf(
                "Change in Word : %d of input seems to have no effect \n", i);
            res = 1;
        }

        // reset old value
        h_inBuffer[i] = i;

        if ((i % (INPUT_BLOCK_SIZE_B / 4 * NB_INPUT_BLOCK)) == 0)
        {
            ctr++;
            printf("\r");
            printf("[ %04d / %04d ]", ctr, NB_THREADS * NB_THREADS_BLOCKS);
        }
    }
    if (res == 0)
        printf("\nTest Completness passed ! \n");


    // free all buffer host and device
    cudaFreeHost(h_inBuffer);
    cudaFreeHost(h_outBuffer);
    cudaFree(d_inBuffer);
    cudaFree(d_outBuffer);
}


void Print_Param(unsigned imax)
{
    printf("\n");
    printf("Numbers of Threads PER BLOCK            NB_THREADS           %u \n",
        NB_THREADS);
    printf("Numbers of Threads Blocks               NB_THREADS_BLOCKS    %u \n",
        NB_THREADS_BLOCKS);
    printf("\n");
    printf("Input block size of Keccak (in Byte)    INPUT_BLOCK_SIZE_B   %u \n",
        INPUT_BLOCK_SIZE_B);
    printf("Output block size of Keccak (in Byte)   OUTPUT_BLOCK_SIZE_B  %u \n",
        OUTPUT_BLOCK_SIZE_B);
    printf("\n");
    printf("NB of input blocks in by Threads        NB_INPUT_BLOCK       %u \n",
        NB_INPUT_BLOCK);
    printf("Numbers of Streams                      NB_STREAMS           %u \n",
        NB_STREAMS);
    printf("\n");
    printf(
        "NB of 2 stage Threads                   NB_SCND_STAGE_THREADS        "
        "%u \n",
        NB_SCND_STAGE_THREADS);
    printf(
        "NB of in blocks 2 stage                 NB_INPUT_BLOCK_SNCD_STAGE    "
        "%u \n",
        NB_INPUT_BLOCK_SNCD_STAGE);
    printf("\nNumber of iterations %u\n", imax);
    printf("\n");
}


void Device_Info(void)
{
    struct cudaDeviceProp deviceProp;
    int device = 0;
    // Get properties and verify device 0 supports mapped memory
    cudaGetDeviceProperties(&deviceProp, 0);
    checkCUDAError("cudaGetDeviceProperties");

    printf("\n");
    // List hardware specs
    printf("%d - name:                    %s\n", device, deviceProp.name);
    printf("%d - MultiProcessorCount:     %d \n", device,
        deviceProp.multiProcessorCount);
    printf("%d - compute capability:      %d.%d\n", device, deviceProp.major,
        deviceProp.minor);
    printf("%d - clockRate                %d kilohertz\n", device,
        deviceProp.clockRate);
    printf("\n");
    printf("%d - totalGlobalMem:          %u bytes ( %.2f Gbytes)\n", device,
        (unsigned)deviceProp.totalGlobalMem,
        deviceProp.totalGlobalMem / (float)(1024 * 1024 * 1024));
    printf("%d - sharedMemPerBlock:       %d bytes ( %.2f Kbytes)\n", device,
        (int)deviceProp.sharedMemPerBlock,
        deviceProp.sharedMemPerBlock / (float)1024);
    printf(
        "%d - regsPerBlock:            %d\n", device, deviceProp.regsPerBlock);
    printf("\n");
    printf("%d - warpSize:                %d\n", device, deviceProp.warpSize);
    printf(
        "%d - memPitch:                %d\n", device, (int)deviceProp.memPitch);
    printf("%d - maxThreadsPerBlock:      %d\n", device,
        deviceProp.maxThreadsPerBlock);

    /*
    printf( "%d - maxThreadsDim[0]:        %d\n" ,device
    ,deviceProp.maxThreadsDim[0] ); printf( "%d - maxThreadsDim[1]:        %d\n"
    ,device ,deviceProp.maxThreadsDim[1] ); printf( "%d - maxThreadsDim[2]:
    %d\n" ,device ,deviceProp.maxThreadsDim[2] ); printf( "%d - maxGridSize[0]:
    %d\n" ,device ,deviceProp.maxGridSize[0] ); printf( "%d - maxGridSize[1]:
    %d\n" ,device ,deviceProp.maxGridSize[1] ); printf( "%d - maxGridSize[2]:
    %d\n" ,device ,deviceProp.maxGridSize[2] ); printf( "%d - totalConstMem: %d
    bytes ( %.2f Kbytes)\n" ,device ,deviceProp.totalConstMem
    ,deviceProp.totalConstMem / (float) 1024 );


    printf( "%d - textureAlignment         %d\n\n" ,device
    ,deviceProp.textureAlignment );
    */
}
#include <stdio.h>
