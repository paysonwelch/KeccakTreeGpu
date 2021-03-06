#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//#include "KeccakTreeGPU.h"

#include "KeccakF.h"
#include "KeccakTree.h"


extern "C" __host__ void checkCUDAError(const char* msg)
{
    cudaError_t err = cudaGetLastError();
    if (cudaSuccess != err)
    {
        fprintf(stderr, "Cuda error: %s: %s.\n", msg, cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }
}

// GPU constants
__constant__ unsigned KeccakF_RoundConstants[22] = {(unsigned)0x00000001,
    (unsigned)0x00008082, (unsigned)0x0000808a, (unsigned)0x80008000,
    (unsigned)0x0000808b, (unsigned)0x80000001, (unsigned)0x80008081,
    (unsigned)0x00008009, (unsigned)0x0000008a, (unsigned)0x00000088,
    (unsigned)0x80008009, (unsigned)0x8000000a, (unsigned)0x8000808b,
    (unsigned)0x0000008b, (unsigned)0x00008089, (unsigned)0x00008003,
    (unsigned)0x00008002, (unsigned)0x00000080, (unsigned)0x0000800a,
    (unsigned)0x8000000a, (unsigned)0x80008081, (unsigned)0x00008080};

// host constants
unsigned KeccakF_RoundConstants_h[22] = {(unsigned)0x00000001,
    (unsigned)0x00008082, (unsigned)0x0000808a, (unsigned)0x80008000,
    (unsigned)0x0000808b, (unsigned)0x80000001, (unsigned)0x80008081,
    (unsigned)0x00008009, (unsigned)0x0000008a, (unsigned)0x00000088,
    (unsigned)0x80008009, (unsigned)0x8000000a, (unsigned)0x8000808b,
    (unsigned)0x0000008b, (unsigned)0x00008089, (unsigned)0x00008003,
    (unsigned)0x00008002, (unsigned)0x00000080, (unsigned)0x0000800a,
    (unsigned)0x8000000a, (unsigned)0x80008081, (unsigned)0x00008080};

// Device (GPU) Keccak-f function implementation
// unrolled
__device__ void KeccakFunr(unsigned* state)
{
    unsigned int round;  // try to avoid to many registers
    unsigned BC[5];
    unsigned temp;

    for (round = 0; round < cKeccakNumberOfRounds; ++round)
    {
        {
            // Theta
            BC[0] = state[0] ^ state[5] ^ state[10] ^ state[15] ^ state[20];
            BC[1] = state[1] ^ state[6] ^ state[11] ^ state[16] ^ state[21];
            BC[2] = state[2] ^ state[7] ^ state[12] ^ state[17] ^ state[22];
            BC[3] = state[3] ^ state[8] ^ state[13] ^ state[18] ^ state[23];
            BC[4] = state[4] ^ state[9] ^ state[14] ^ state[19] ^ state[24];

            temp = BC[4] ^ ROL32(BC[1], 1);  // x=0
            state[0] ^= temp;
            state[5] ^= temp;
            state[10] ^= temp;
            state[15] ^= temp;
            state[20] ^= temp;
            temp = BC[0] ^ ROL32(BC[2], 1);  // x=1
            state[1] ^= temp;
            state[6] ^= temp;
            state[11] ^= temp;
            state[16] ^= temp;
            state[21] ^= temp;
            temp = BC[1] ^ ROL32(BC[3], 1);  // x=2
            state[2] ^= temp;
            state[7] ^= temp;
            state[12] ^= temp;
            state[17] ^= temp;
            state[22] ^= temp;
            temp = BC[2] ^ ROL32(BC[4], 1);  // x=3
            state[3] ^= temp;
            state[8] ^= temp;
            state[13] ^= temp;
            state[18] ^= temp;
            state[23] ^= temp;
            temp = BC[3] ^ ROL32(BC[0], 1);  // x=4
            state[4] ^= temp;
            state[9] ^= temp;
            state[14] ^= temp;
            state[19] ^= temp;
            state[24] ^= temp;
        }  // end Theta

        {
            // Rho Pi
            temp = state[1];
            BC[0] = state[10];
            state[10] = ROL32(temp, 1);
            temp = BC[0];  // x=0
            BC[0] = state[7];
            state[7] = ROL32(temp, 3);
            temp = BC[0];
            BC[0] = state[11];
            state[11] = ROL32(temp, 6);
            temp = BC[0];
            BC[0] = state[17];
            state[17] = ROL32(temp, 10);
            temp = BC[0];
            BC[0] = state[18];
            state[18] = ROL32(temp, 15);
            temp = BC[0];
            BC[0] = state[3];
            state[3] = ROL32(temp, 21);
            temp = BC[0];  // x=5
            BC[0] = state[5];
            state[5] = ROL32(temp, 28);
            temp = BC[0];
            BC[0] = state[16];
            state[16] = ROL32(temp, 4);
            temp = BC[0];
            BC[0] = state[8];
            state[8] = ROL32(temp, 13);
            temp = BC[0];
            BC[0] = state[21];
            state[21] = ROL32(temp, 23);
            temp = BC[0];
            BC[0] = state[24];
            state[24] = ROL32(temp, 2);
            temp = BC[0];  // x=10
            BC[0] = state[4];
            state[4] = ROL32(temp, 14);
            temp = BC[0];
            BC[0] = state[15];
            state[15] = ROL32(temp, 27);
            temp = BC[0];
            BC[0] = state[23];
            state[23] = ROL32(temp, 9);
            temp = BC[0];
            BC[0] = state[19];
            state[19] = ROL32(temp, 24);
            temp = BC[0];
            BC[0] = state[13];
            state[13] = ROL32(temp, 8);
            temp = BC[0];  // x=15
            BC[0] = state[12];
            state[12] = ROL32(temp, 25);
            temp = BC[0];
            BC[0] = state[2];
            state[2] = ROL32(temp, 11);
            temp = BC[0];
            BC[0] = state[20];
            state[20] = ROL32(temp, 30);
            temp = BC[0];
            BC[0] = state[14];
            state[14] = ROL32(temp, 18);
            temp = BC[0];
            BC[0] = state[22];
            state[22] = ROL32(temp, 7);
            temp = BC[0];  // x=20
            BC[0] = state[9];
            state[9] = ROL32(temp, 29);
            temp = BC[0];
            BC[0] = state[6];
            state[6] = ROL32(temp, 20);
            temp = BC[0];
            BC[0] = state[1];
            state[1] = ROL32(temp, 12);
            temp = BC[0];  // x=23
        }                  // end Rho Pi

        {
            //	Chi
            BC[0] = state[0];
            BC[1] = state[1];
            BC[2] = state[2];
            BC[3] = state[3];
            BC[4] = state[4];
            state[0] = BC[0] ^ ((~BC[1]) & BC[2]);
            state[1] = BC[1] ^ ((~BC[2]) & BC[3]);
            state[2] = BC[2] ^ ((~BC[3]) & BC[4]);
            state[3] = BC[3] ^ ((~BC[4]) & BC[0]);
            state[4] = BC[4] ^ ((~BC[0]) & BC[1]);
            BC[0] = state[5];
            BC[1] = state[6];
            BC[2] = state[7];
            BC[3] = state[8];
            BC[4] = state[9];
            state[5] = BC[0] ^ ((~BC[1]) & BC[2]);
            state[6] = BC[1] ^ ((~BC[2]) & BC[3]);
            state[7] = BC[2] ^ ((~BC[3]) & BC[4]);
            state[8] = BC[3] ^ ((~BC[4]) & BC[0]);
            state[9] = BC[4] ^ ((~BC[0]) & BC[1]);
            BC[0] = state[10];
            BC[1] = state[11];
            BC[2] = state[12];
            BC[3] = state[13];
            BC[4] = state[14];
            state[10] = BC[0] ^ ((~BC[1]) & BC[2]);
            state[11] = BC[1] ^ ((~BC[2]) & BC[3]);
            state[12] = BC[2] ^ ((~BC[3]) & BC[4]);
            state[13] = BC[3] ^ ((~BC[4]) & BC[0]);
            state[14] = BC[4] ^ ((~BC[0]) & BC[1]);
            BC[0] = state[15];
            BC[1] = state[16];
            BC[2] = state[17];
            BC[3] = state[18];
            BC[4] = state[19];
            state[15] = BC[0] ^ ((~BC[1]) & BC[2]);
            state[16] = BC[1] ^ ((~BC[2]) & BC[3]);
            state[17] = BC[2] ^ ((~BC[3]) & BC[4]);
            state[18] = BC[3] ^ ((~BC[4]) & BC[0]);
            state[19] = BC[4] ^ ((~BC[0]) & BC[1]);
            BC[0] = state[20];
            BC[1] = state[21];
            BC[2] = state[22];
            BC[3] = state[23];
            BC[4] = state[24];
            state[20] = BC[0] ^ ((~BC[1]) & BC[2]);
            state[21] = BC[1] ^ ((~BC[2]) & BC[3]);
            state[22] = BC[2] ^ ((~BC[3]) & BC[4]);
            state[23] = BC[3] ^ ((~BC[4]) & BC[0]);
            state[24] = BC[4] ^ ((~BC[0]) & BC[1]);
        }  // end Chi

        //	Iota
        state[0] ^= KeccakF_RoundConstants[round];
    }
}
// end unrolled

// Host Keccak-f function (pb with using the same constants between host and
// device) unrolled
__host__ void KeccakFunr_h(unsigned* state)
{
    unsigned int round;  // try to avoid to many registers
    unsigned BC[5];
    unsigned temp;

    for (round = 0; round < cKeccakNumberOfRounds; ++round)
    {
        {
            // Theta
            BC[0] = state[0] ^ state[5] ^ state[10] ^ state[15] ^ state[20];
            BC[1] = state[1] ^ state[6] ^ state[11] ^ state[16] ^ state[21];
            BC[2] = state[2] ^ state[7] ^ state[12] ^ state[17] ^ state[22];
            BC[3] = state[3] ^ state[8] ^ state[13] ^ state[18] ^ state[23];
            BC[4] = state[4] ^ state[9] ^ state[14] ^ state[19] ^ state[24];

            temp = BC[4] ^ ROL32(BC[1], 1);  // x=0
            state[0] ^= temp;
            state[5] ^= temp;
            state[10] ^= temp;
            state[15] ^= temp;
            state[20] ^= temp;
            temp = BC[0] ^ ROL32(BC[2], 1);  // x=1
            state[1] ^= temp;
            state[6] ^= temp;
            state[11] ^= temp;
            state[16] ^= temp;
            state[21] ^= temp;
            temp = BC[1] ^ ROL32(BC[3], 1);  // x=2
            state[2] ^= temp;
            state[7] ^= temp;
            state[12] ^= temp;
            state[17] ^= temp;
            state[22] ^= temp;
            temp = BC[2] ^ ROL32(BC[4], 1);  // x=3
            state[3] ^= temp;
            state[8] ^= temp;
            state[13] ^= temp;
            state[18] ^= temp;
            state[23] ^= temp;
            temp = BC[3] ^ ROL32(BC[0], 1);  // x=4
            state[4] ^= temp;
            state[9] ^= temp;
            state[14] ^= temp;
            state[19] ^= temp;
            state[24] ^= temp;
        }  // end Theta

        {
            // Rho Pi
            temp = state[1];
            BC[0] = state[10];
            state[10] = ROL32(temp, 1);
            temp = BC[0];  // x=0
            BC[0] = state[7];
            state[7] = ROL32(temp, 3);
            temp = BC[0];
            BC[0] = state[11];
            state[11] = ROL32(temp, 6);
            temp = BC[0];
            BC[0] = state[17];
            state[17] = ROL32(temp, 10);
            temp = BC[0];
            BC[0] = state[18];
            state[18] = ROL32(temp, 15);
            temp = BC[0];
            BC[0] = state[3];
            state[3] = ROL32(temp, 21);
            temp = BC[0];  // x=5
            BC[0] = state[5];
            state[5] = ROL32(temp, 28);
            temp = BC[0];
            BC[0] = state[16];
            state[16] = ROL32(temp, 4);
            temp = BC[0];
            BC[0] = state[8];
            state[8] = ROL32(temp, 13);
            temp = BC[0];
            BC[0] = state[21];
            state[21] = ROL32(temp, 23);
            temp = BC[0];
            BC[0] = state[24];
            state[24] = ROL32(temp, 2);
            temp = BC[0];  // x=10
            BC[0] = state[4];
            state[4] = ROL32(temp, 14);
            temp = BC[0];
            BC[0] = state[15];
            state[15] = ROL32(temp, 27);
            temp = BC[0];
            BC[0] = state[23];
            state[23] = ROL32(temp, 9);
            temp = BC[0];
            BC[0] = state[19];
            state[19] = ROL32(temp, 24);
            temp = BC[0];
            BC[0] = state[13];
            state[13] = ROL32(temp, 8);
            temp = BC[0];  // x=15
            BC[0] = state[12];
            state[12] = ROL32(temp, 25);
            temp = BC[0];
            BC[0] = state[2];
            state[2] = ROL32(temp, 11);
            temp = BC[0];
            BC[0] = state[20];
            state[20] = ROL32(temp, 30);
            temp = BC[0];
            BC[0] = state[14];
            state[14] = ROL32(temp, 18);
            temp = BC[0];
            BC[0] = state[22];
            state[22] = ROL32(temp, 7);
            temp = BC[0];  // x=20
            BC[0] = state[9];
            state[9] = ROL32(temp, 29);
            temp = BC[0];
            BC[0] = state[6];
            state[6] = ROL32(temp, 20);
            temp = BC[0];
            BC[0] = state[1];
            state[1] = ROL32(temp, 12);
            temp = BC[0];  // x=23
        }                  // end Rho Pi

        {
            //	Chi
            BC[0] = state[0];
            BC[1] = state[1];
            BC[2] = state[2];
            BC[3] = state[3];
            BC[4] = state[4];
            state[0] = BC[0] ^ ((~BC[1]) & BC[2]);
            state[1] = BC[1] ^ ((~BC[2]) & BC[3]);
            state[2] = BC[2] ^ ((~BC[3]) & BC[4]);
            state[3] = BC[3] ^ ((~BC[4]) & BC[0]);
            state[4] = BC[4] ^ ((~BC[0]) & BC[1]);
            BC[0] = state[5];
            BC[1] = state[6];
            BC[2] = state[7];
            BC[3] = state[8];
            BC[4] = state[9];
            state[5] = BC[0] ^ ((~BC[1]) & BC[2]);
            state[6] = BC[1] ^ ((~BC[2]) & BC[3]);
            state[7] = BC[2] ^ ((~BC[3]) & BC[4]);
            state[8] = BC[3] ^ ((~BC[4]) & BC[0]);
            state[9] = BC[4] ^ ((~BC[0]) & BC[1]);
            BC[0] = state[10];
            BC[1] = state[11];
            BC[2] = state[12];
            BC[3] = state[13];
            BC[4] = state[14];
            state[10] = BC[0] ^ ((~BC[1]) & BC[2]);
            state[11] = BC[1] ^ ((~BC[2]) & BC[3]);
            state[12] = BC[2] ^ ((~BC[3]) & BC[4]);
            state[13] = BC[3] ^ ((~BC[4]) & BC[0]);
            state[14] = BC[4] ^ ((~BC[0]) & BC[1]);
            BC[0] = state[15];
            BC[1] = state[16];
            BC[2] = state[17];
            BC[3] = state[18];
            BC[4] = state[19];
            state[15] = BC[0] ^ ((~BC[1]) & BC[2]);
            state[16] = BC[1] ^ ((~BC[2]) & BC[3]);
            state[17] = BC[2] ^ ((~BC[3]) & BC[4]);
            state[18] = BC[3] ^ ((~BC[4]) & BC[0]);
            state[19] = BC[4] ^ ((~BC[0]) & BC[1]);
            BC[0] = state[20];
            BC[1] = state[21];
            BC[2] = state[22];
            BC[3] = state[23];
            BC[4] = state[24];
            state[20] = BC[0] ^ ((~BC[1]) & BC[2]);
            state[21] = BC[1] ^ ((~BC[2]) & BC[3]);
            state[22] = BC[2] ^ ((~BC[3]) & BC[4]);
            state[23] = BC[3] ^ ((~BC[4]) & BC[0]);
            state[24] = BC[4] ^ ((~BC[0]) & BC[1]);
        }  // end Chi

        //	Iota
        state[0] ^= KeccakF_RoundConstants_h[round];
    }
}
// end unrolled

// Keccak final node hashing results of previous nodes in sequential mode
__host__ void Keccak_top_GPU(
    unsigned* Kstate, unsigned* inBuffer, int block_number)
{
    int ind_word, k;

    for (k = 0; k < block_number; k++)
    {
        for (ind_word = 0; ind_word < OUTPUT_BLOCK_SIZE_B / 4; ind_word++)
        {
            Kstate[ind_word] ^=
                inBuffer[ind_word + k * OUTPUT_BLOCK_SIZE_B / 4];
        }
        KeccakFunr_h(Kstate);

        // if(k%192==0)
        // printf("Kstate %08x %08x \n", Kstate[0], Kstate[1]);
    }
    // printf("\n\n");
}

//************************************************************************
// kernel implementaing hash function, hashing NB_INPUT_BLOCK (of 256 bits)
//
__global__ void ker_Keccak(unsigned* d_inBuffer, unsigned* d_outBuffer)
{
    int ind_word, k;
    unsigned Kstate[25];

    // zeroize the state
    for (ind_word = 0; ind_word < 25; ind_word++)
    {
        Kstate[ind_word] = 0;
    }

    for (k = 0; k < NB_INPUT_BLOCK; k++)
    {
        // xor input into state
        for (ind_word = 0; ind_word < (INPUT_BLOCK_SIZE_B / 4); ind_word++)
        {
            Kstate[ind_word] ^=
                d_inBuffer[threadIdx.x + ind_word * NB_THREADS +
                           k * NB_THREADS * INPUT_BLOCK_SIZE_B / 4 +
                           blockIdx.x * NB_THREADS * INPUT_BLOCK_SIZE_B / 4 *
                               NB_INPUT_BLOCK];
        }
        // apply GPU Keccak permutation
        KeccakFunr(Kstate);
    }

    // output hash in buffer
    for (ind_word = 0; ind_word < OUTPUT_BLOCK_SIZE_B / 4; ind_word++)
    {
        d_outBuffer[threadIdx.x + ind_word * NB_THREADS +
                    blockIdx.x * NB_THREADS * OUTPUT_BLOCK_SIZE_B / 4] =
            Kstate[ind_word];
    }
}


//******************************************************
// Implementing 2 stages treehash
//******************************************************
__global__ void ker_Keccak_2stg(unsigned* d_inBuffer, unsigned* d_outBuffer)
{
    int ind_word, k;
    unsigned Kstate[25];

    __shared__ unsigned SharedBuffer[2 * OUTPUT_BLOCK_SIZE_B / 4 * NB_THREADS];

    // zeroize the state
    for (ind_word = 0; ind_word < 25; ind_word++)
    {
        Kstate[ind_word] = 0;
    }

    for (k = 0; k < NB_INPUT_BLOCK; k++)
    {
        // xor input into state
        for (ind_word = 0; ind_word < (INPUT_BLOCK_SIZE_B / 4); ind_word++)
        {
            Kstate[ind_word] ^=
                d_inBuffer[threadIdx.x + ind_word * NB_THREADS +
                           k * NB_THREADS * INPUT_BLOCK_SIZE_B / 4 +
                           blockIdx.x * NB_THREADS * INPUT_BLOCK_SIZE_B / 4 *
                               NB_INPUT_BLOCK];
        }
        // apply GPU Keccak permutation
        KeccakFunr(Kstate);
    }

    // output hash in SharedBuffer
    for (ind_word = 0; ind_word < OUTPUT_BLOCK_SIZE_B / 4; ind_word++)
    {
        // printf("Kstate[%02u] = %08x",ind_word,Kstate[ind_word] );
        SharedBuffer[threadIdx.x + ind_word * NB_THREADS] = Kstate[ind_word];
    }

    // need to squeeze to produce more hash output
    KeccakFunr(Kstate);

    for (ind_word = 0; ind_word < OUTPUT_BLOCK_SIZE_B / 4; ind_word++)
    {
        SharedBuffer[threadIdx.x + ind_word * NB_THREADS +
                     NB_THREADS * OUTPUT_BLOCK_SIZE_B / 4] = Kstate[ind_word];
    }

    // syncthreads should be mandatory to ensure that all writes to sharedmem is
    // done before reading
    __syncthreads();

    //***************
    // Second stage

    if (threadIdx.x < NB_SCND_STAGE_THREADS)  // Only first
                                              // NB_SCND_STAGE_THREADS threads
                                              // run this part
    {
        // zeroize the state
        for (ind_word = 0; ind_word < 25; ind_word++)
        {
            Kstate[ind_word] = 0;
        }

        // number of input block per thread is now NB_INPUT_BLOCK_SNCD_STAGE =
        // 2* NB_THREADS/ NB_SNCD_STAGE_THREADS
        //
        for (k = 0; k < NB_INPUT_BLOCK_SNCD_STAGE; k++)
        {
            // xor input into state
            for (ind_word = 0; ind_word < INPUT_BLOCK_SIZE_B / 4; ind_word++)
            {
                Kstate[ind_word] ^=
                    SharedBuffer[threadIdx.x +
                                 ind_word * NB_SCND_STAGE_THREADS +
                                 k * NB_SCND_STAGE_THREADS *
                                     INPUT_BLOCK_SIZE_B / 4];
            }
            // apply Keccak permutation
            KeccakFunr(Kstate);
        }

        // output hash in output buffer
        for (ind_word = 0; ind_word < OUTPUT_BLOCK_SIZE_B / 4; ind_word++)
        {
            d_outBuffer[threadIdx.x + ind_word * NB_SCND_STAGE_THREADS +
                        blockIdx.x * NB_SCND_STAGE_THREADS * 2 *
                            OUTPUT_BLOCK_SIZE_B / 4] = Kstate[ind_word];
        }

        // need to squeeze to produce more hash output
        KeccakFunr(Kstate);

        for (ind_word = 0; ind_word < OUTPUT_BLOCK_SIZE_B / 4; ind_word++)
        {
            d_outBuffer[threadIdx.x + ind_word * NB_SCND_STAGE_THREADS +
                        NB_SCND_STAGE_THREADS * OUTPUT_BLOCK_SIZE_B / 4 +
                        blockIdx.x * NB_SCND_STAGE_THREADS * 2 *
                            OUTPUT_BLOCK_SIZE_B / 4] = Kstate[ind_word];
        }

    }  // end if threadIdx.x < NB_SCND_STAGE_THREADS
}

//*************************
// Keccak Stream cipher mode
// d_KeyNonce_inBuffer must point to a 256 bits Key + 256 bits Nonce (random)
//*************************
__global__ void ker_Keccak_SCipher(
    unsigned* d_KeyNonce_inBuffer, unsigned* d_outBuffer)
{
    int ind_word, k;
    unsigned Kstate[25];

    // used shared memory to load only once the Key and Nonce
    __shared__ unsigned Key[8];    // 8 32b words for a secret key
    __shared__ unsigned Nonce[8];  //

    // load Key and Nonce in shared mem assuming that NB_THREADS used will be >
    // 8
    if (threadIdx.x < 8)
    {
        Key[threadIdx.x] = d_KeyNonce_inBuffer[threadIdx.x];
        Nonce[threadIdx.x] = d_KeyNonce_inBuffer[threadIdx.x + 8];
    }
    __syncthreads();

    // zeroize the state
    for (ind_word = 0; ind_word < 25; ind_word++)
    {
        Kstate[ind_word] = 0;
    }

    // input the key
    // xor Key into state from shared mem
    for (ind_word = 0; ind_word < 8; ind_word++)
    {
        Kstate[ind_word] ^= Key[ind_word];
    }
    // apply GPU Keccak permutation
    KeccakFunr(Kstate);

    // input the Nonce
    // xor Key into state from shared mem
    for (ind_word = 0; ind_word < 8; ind_word++)
    {
        Kstate[ind_word] ^= Nonce[ind_word];
    }
    // apply GPU Keccak permutation
    KeccakFunr(Kstate);

    // input the threadIdx.x AND blockIdx.x to have different KeyStreams
    Kstate[0] ^= threadIdx.x;
    Kstate[1] ^= blockIdx.x;
    KeccakFunr(Kstate);


    for (k = 0; k < SC_NB_OUTPUT_BLOCK; k++)
    {
        // output KeyStreams in d_outBuffer
        for (ind_word = 0; ind_word < OUTPUT_BLOCK_SIZE_B / 4; ind_word++)
        {
            d_outBuffer[threadIdx.x + ind_word * NB_THREADS +
                        k * NB_THREADS * OUTPUT_BLOCK_SIZE_B / 4 +
                        blockIdx.x * NB_THREADS * OUTPUT_BLOCK_SIZE_B / 4 *
                            SC_NB_OUTPUT_BLOCK] = Kstate[ind_word];
        }
        // Squeeze the state !
        KeccakFunr(Kstate);
    }
}


//********************************************************************************

//************************
// First Tree mode
// data to be hashed is in h_inBuffer
// output chaining values hashes are copied to h_outBuffer
//************************
extern "C" __host__ void KeccakTreeGPU(unsigned* h_inBuffer,
    unsigned* d_inBuffer, unsigned* h_outBuffer, unsigned* d_outBuffer)
{
    // copy host to device
    cudaMemcpy(d_inBuffer, h_inBuffer,
        INPUT_BLOCK_SIZE_B * NB_THREADS * NB_INPUT_BLOCK * NB_THREADS_BLOCKS,
        cudaMemcpyHostToDevice);
    // checkCUDAError(" Memcpy htd");

    // exec kernels

    ker_Keccak<<<NB_THREADS_BLOCKS, NB_THREADS>>>(d_inBuffer, d_outBuffer);
    // checkCUDAError(" ker_keccak");

    // cudaThreadSynchronize(); // should be useless when followed by cudaMemcpy

    // copy back device to host
    cudaMemcpy(h_outBuffer, d_outBuffer,
        OUTPUT_BLOCK_SIZE_B * NB_THREADS * NB_THREADS_BLOCKS,
        cudaMemcpyDeviceToHost);
    // checkCUDAError(" Memcpy dth");
}


//***********************************************************
// Overlap GPU kernel computation and CPU top node computation
//***********************************************************
extern "C" __host__ void KeccakTreeGPU_overlapCPU(unsigned* h_inBuffer,
    unsigned* d_inBuffer, unsigned* h_outBuffer, unsigned* d_outBuffer,
    unsigned* Kstate)
{
    // copy input data host to device
    cudaMemcpy(d_inBuffer, h_inBuffer,
        INPUT_BLOCK_SIZE_B * NB_THREADS * NB_INPUT_BLOCK * NB_THREADS_BLOCKS,
        cudaMemcpyHostToDevice);
    // checkCUDAError(" Memcpy htd");

    // execute cuda kernels (hashing data in each node)

    ker_Keccak<<<NB_THREADS_BLOCKS, NB_THREADS>>>(d_inBuffer, d_outBuffer);
    // checkCUDAError(" ker_keccak");

    // compute previous data on CPU Overlapping
    Keccak_top_GPU(Kstate, h_outBuffer, NB_THREADS * NB_THREADS_BLOCKS);

    // copy back device to host
    cudaMemcpy(h_outBuffer, d_outBuffer,
        OUTPUT_BLOCK_SIZE_B * NB_THREADS * NB_THREADS_BLOCKS,
        cudaMemcpyDeviceToHost);
    // checkCUDAError(" Memcpy dth");
}


//***********************************************************
// Split computation over NB_STREAMS several kernels
//***********************************************************
extern "C" __host__ void KeccakTreeGPU_Split(unsigned* h_inBuffer,
    unsigned* d_inBuffer, unsigned* h_outBuffer, unsigned* d_outBuffer)
{
    unsigned int inOffset;
    unsigned int outOffset;
    int s;

    // copy host to device
    cudaMemcpy(d_inBuffer, h_inBuffer,
        INPUT_BLOCK_SIZE_B * NB_THREADS * NB_INPUT_BLOCK * NB_THREADS_BLOCKS,
        cudaMemcpyHostToDevice);
    // checkCUDAError(" Memcpy htd");

    // Offset (for 32b Word )
    inOffset = INPUT_BLOCK_SIZE_B / 4 * NB_THREADS * NB_INPUT_BLOCK *
               NB_THREADS_BLOCKS / NB_STREAMS;
    outOffset =
        OUTPUT_BLOCK_SIZE_B / 4 * NB_THREADS * NB_THREADS_BLOCKS / NB_STREAMS;

    // exec kernels
    for (s = 0; s < NB_STREAMS; s++)
    {
        ker_Keccak<<<NB_THREADS_BLOCKS / NB_STREAMS, NB_THREADS>>>(
            d_inBuffer + s * inOffset, d_outBuffer + s * outOffset);
    }  // checkCUDAError(" ker_keccak");


    // copy back device to host
    cudaMemcpy(h_outBuffer, d_outBuffer,
        OUTPUT_BLOCK_SIZE_B * NB_THREADS * NB_THREADS_BLOCKS,
        cudaMemcpyDeviceToHost);
    // checkCUDAError(" Memcpy dth");
}


//***********************************************************
// Split computation over several STREAMS
//***********************************************************
extern "C" __host__ void KeccakTreeGPU_Stream(unsigned* h_inBuffer,
    unsigned* d_inBuffer, unsigned* h_outBuffer, unsigned* d_outBuffer)
{
    unsigned int inOffset;
    unsigned int outOffset;
    int s;

    cudaStream_t stream[NB_STREAMS];

    for (s = 0; s < NB_STREAMS; s++)
    {
        cudaStreamCreate(&stream[s]);
    }
    checkCUDAError(" StreamCreate");

    // Offset (for 32b Word )
    inOffset = INPUT_BLOCK_SIZE_B / 4 * NB_THREADS * NB_INPUT_BLOCK *
               NB_THREADS_BLOCKS / NB_STREAMS;
    outOffset =
        OUTPUT_BLOCK_SIZE_B / 4 * NB_THREADS * NB_THREADS_BLOCKS / NB_STREAMS;

    // exec kernels
    for (s = 0; s < NB_STREAMS; s++)
    {
        // copy host to device, PARTIAL
        cudaMemcpyAsync(d_inBuffer + s * inOffset, h_inBuffer + s * inOffset,
            inOffset * 4, cudaMemcpyHostToDevice, stream[s]);
        checkCUDAError(" MemcpyAsync htd");

        ker_Keccak<<<NB_THREADS_BLOCKS / NB_STREAMS, NB_THREADS, 0,
            stream[s]>>>(
            d_inBuffer + s * inOffset, d_outBuffer + s * outOffset);
    }
    checkCUDAError(" ker_keccak");


    // copy back device to host
    cudaMemcpy(h_outBuffer, d_outBuffer,
        OUTPUT_BLOCK_SIZE_B * NB_THREADS * NB_THREADS_BLOCKS,
        cudaMemcpyDeviceToHost);
    // checkCUDAError(" Memcpy dth");


    for (s = 0; s < NB_STREAMS; s++)
    {
        cudaStreamDestroy(stream[s]);
    }
    checkCUDAError(" StreamDestroy");
}


//***********************************************************
// Split computation over several Cuda STREAMS, and overlap with CPU
//***********************************************************
extern "C" __host__ void KeccakTreeGPU_Stream_OverlapCPU(unsigned* h_inBuffer,
    unsigned* d_inBuffer, unsigned* h_outBuffer, unsigned* d_outBuffer,
    unsigned* Kstate)
{
    unsigned int inOffset;
    unsigned int outOffset;
    int s;

    cudaStream_t stream[NB_STREAMS];

    for (s = 0; s < NB_STREAMS; s++)
    {
        cudaStreamCreate(&stream[s]);
    }
    checkCUDAError(" StreamCreate");


    // Offset (for 32b Word )
    inOffset = INPUT_BLOCK_SIZE_B / 4 * NB_THREADS * NB_INPUT_BLOCK *
               NB_THREADS_BLOCKS / NB_STREAMS;
    outOffset =
        OUTPUT_BLOCK_SIZE_B / 4 * NB_THREADS * NB_THREADS_BLOCKS / NB_STREAMS;


    // exec kernels
    for (s = 0; s < NB_STREAMS; s++)
    {
        // copy host to device, PARTIAL
        cudaMemcpyAsync(d_inBuffer + s * inOffset, h_inBuffer + s * inOffset,
            inOffset * 4, cudaMemcpyHostToDevice, stream[s]);
        // checkCUDAError(" MemcpyAsync htd");

        ker_Keccak<<<NB_THREADS_BLOCKS / NB_STREAMS, NB_THREADS, 0,
            stream[s]>>>(
            d_inBuffer + s * inOffset, d_outBuffer + s * outOffset);
    }
    // checkCUDAError(" ker_keccak");

    Keccak_top_GPU(Kstate, h_outBuffer, NB_THREADS * NB_THREADS_BLOCKS);

    // cudaThreadSynchronize(); // should be useless when followed by cudaMemcpy

    // copy back device to host
    cudaMemcpy(h_outBuffer, d_outBuffer,
        OUTPUT_BLOCK_SIZE_B * NB_THREADS * NB_THREADS_BLOCKS,
        cudaMemcpyDeviceToHost);
    // checkCUDAError(" Memcpy dth");


    for (s = 0; s < NB_STREAMS; s++)
    {
        cudaStreamDestroy(stream[s]);
    }
    checkCUDAError(" StreamDestroy");
}


//******************************************************
// 2 stages Stage
//******************************************************

extern "C" __host__ void KeccakTreeGPU_2stg(unsigned* h_inBuffer,
    unsigned* d_inBuffer, unsigned* h_outBuffer, unsigned* d_outBuffer)
{
    // copy host to device
    cudaMemcpy(d_inBuffer, h_inBuffer,
        INPUT_BLOCK_SIZE_B * NB_THREADS * NB_INPUT_BLOCK * NB_THREADS_BLOCKS,
        cudaMemcpyHostToDevice);
    // checkCUDAError(" Memcpy htd");

    // exec kernels

    ker_Keccak_2stg<<<NB_THREADS_BLOCKS, NB_THREADS>>>(d_inBuffer, d_outBuffer);
    // checkCUDAError(" ker_keccak_2stg");


    // cudaThreadSynchronize(); // should be useless when followed by cudaMemcpy

    // copy back device to host
    cudaMemcpy(h_outBuffer, d_outBuffer,
        2 * OUTPUT_BLOCK_SIZE_B * NB_SCND_STAGE_THREADS * NB_THREADS_BLOCKS,
        cudaMemcpyDeviceToHost);
    // checkCUDAError(" Memcpy dth");
}


//***********************************************************
// 2 stages +  Split computation over several Cuda STREAMS, and overlap with CPU
//***********************************************************
extern "C" __host__ void KeccakTreeGPU_2stg_Stream_OverlapCPU(
    unsigned* h_inBuffer, unsigned* d_inBuffer, unsigned* h_outBuffer,
    unsigned* d_outBuffer, unsigned* Kstate)
{
    unsigned int inOffset;
    unsigned int outOffset;
    int s;

    cudaStream_t stream[NB_STREAMS];

    for (s = 0; s < NB_STREAMS; s++)
    {
        cudaStreamCreate(&stream[s]);
    }
    checkCUDAError(" StreamCreate");


    // Offset (for 32b Word )
    inOffset = INPUT_BLOCK_SIZE_B / 4 * NB_THREADS * NB_INPUT_BLOCK *
               NB_THREADS_BLOCKS / NB_STREAMS;
    outOffset = OUTPUT_BLOCK_SIZE_B / 4 * 2 * NB_SCND_STAGE_THREADS *
                NB_THREADS_BLOCKS / NB_STREAMS;


    // exec kernels
    for (s = 0; s < NB_STREAMS; s++)
    {
        // copy host to device, PARTIAL
        cudaMemcpyAsync(d_inBuffer + s * inOffset, h_inBuffer + s * inOffset,
            inOffset * 4, cudaMemcpyHostToDevice, stream[s]);
        // checkCUDAError(" MemcpyAsync htd");

        ker_Keccak_2stg<<<NB_THREADS_BLOCKS / NB_STREAMS, NB_THREADS, 0,
            stream[s]>>>(
            d_inBuffer + s * inOffset, d_outBuffer + s * outOffset);
    }
    // checkCUDAError(" ker_keccak_2stg streams");

    Keccak_top_GPU(
        Kstate, h_outBuffer, 2 * NB_SCND_STAGE_THREADS * NB_THREADS_BLOCKS);

    // cudaThreadSynchronize(); // should be useless when followed by cudaMemcpy

    // copy back device to host
    cudaMemcpy(h_outBuffer, d_outBuffer,
        2 * OUTPUT_BLOCK_SIZE_B * NB_SCND_STAGE_THREADS * NB_THREADS_BLOCKS,
        cudaMemcpyDeviceToHost);
    // checkCUDAError(" Memcpy dth");

    for (s = 0; s < NB_STREAMS; s++)
    {
        cudaStreamDestroy(stream[s]);
    }
    checkCUDAError(" StreamDestroy");
}


//***********************************************************
// Keccak in StreamCipher Mode  (Using Cuda Streams)
//***********************************************************
extern "C" __host__ void KeccakSCipherGPU_Stream(unsigned* h_inKeyNonce,
    unsigned* d_inKeyNonce, unsigned* h_outBuffer, unsigned* d_outBuffer)
{
    unsigned int outOffset;
    int s;

    cudaStream_t stream[NB_STREAMS];

    for (s = 0; s < NB_STREAMS; s++)
    {
        cudaStreamCreate(&stream[s]);
    }
    checkCUDAError(" StreamCreate");

    // Offset (for 32b Word )
    outOffset = SC_NB_OUTPUT_BLOCK * OUTPUT_BLOCK_SIZE_B / 4 * NB_THREADS *
                NB_THREADS_BLOCKS / NB_STREAMS;

    // copy Key and Nonce from Host
    cudaMemcpy(d_inKeyNonce, h_inKeyNonce, 2 * (32), cudaMemcpyHostToDevice);
    checkCUDAError(" cudaMemcpy HtD");

    // exec kernels
    for (s = 0; s < NB_STREAMS; s++)
    {
        ker_Keccak_SCipher<<<NB_THREADS_BLOCKS / NB_STREAMS, NB_THREADS, 0,
            stream[s]>>>(d_inKeyNonce, d_outBuffer + s * outOffset);
        checkCUDAError(" ker_keccak_SCipher");

        // copy Device to Host, PARTIAL
        cudaMemcpyAsync(h_outBuffer + s * outOffset,
            d_outBuffer + s * outOffset, outOffset * 4 /*size in Byte*/,
            cudaMemcpyDeviceToHost, stream[s]);

        checkCUDAError(" MemcpyAsync DtH");
    }


    for (s = 0; s < NB_STREAMS; s++)
    {
        cudaStreamDestroy(stream[s]);
    }
    checkCUDAError(" StreamDestroy");
}
