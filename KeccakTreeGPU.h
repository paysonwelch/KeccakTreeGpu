#pragma once

#include "KeccakTree.h"

//************************
// First Tree mode
// data to be hashed is in h_inBuffer
// output chaining values hashes are copied to h_outBuffer
//************************

void KeccakTreeGPU(unsigned* h_inBuffer, unsigned* d_inBuffer,
    unsigned* h_outBuffer, unsigned* d_outBuffer);


// Overlap CPU computation of previous results with computation of present data
// in GPU Kstate is a pointer to Keccak state of final node h_inBuffer contains
// data to be hashed h_outBuffer contains previsous results at the call of the
// function, and present results when the function returns

void KeccakTreeGPU_overlapCPU(unsigned* h_inBuffer, unsigned* d_inBuffer,
    unsigned* h_outBuffer, unsigned* d_outBuffer, unsigned* Kstate);

void KeccakTreeGPU_Split(unsigned* h_inBuffer, unsigned* d_inBuffer,
    unsigned* h_outBuffer, unsigned* d_outBuffer);

void KeccakTreeGPU_Stream(unsigned* h_inBuffer, unsigned* d_inBuffer,
    unsigned* h_outBuffer, unsigned* d_outBuffer);

void KeccakTreeGPU_Stream_OverlapCPU(unsigned* h_inBuffer, unsigned* d_inBuffer,
    unsigned* h_outBuffer, unsigned* d_outBuffer, unsigned* Kstate);

//********************
// 2nd stage Tree mode
//********************
void KeccakTreeGPU_2stg(unsigned* h_inBuffer, unsigned* d_inBuffer,
    unsigned* h_outBuffer, unsigned* d_outBuffer);


// Streams and overlap CPU
void KeccakTreeGPU_2stg_Stream_OverlapCPU(unsigned* h_inBuffer,
    unsigned* d_inBuffer, unsigned* h_outBuffer, unsigned* d_outBuffer,
    unsigned* Kstate);


//******************
// StreamCipher
//******************
// 256 bits key and Nonce should be in h_inKeyNonce
//
// output streams are copied in h_outBuffer

void KeccakSCipherGPU_Stream(unsigned* h_inKeyNonce, unsigned* d_inKeyNonce,
    unsigned* h_outBuffer, unsigned* d_outBuffer);


// error function
void checkCUDAError(const char* msg);

