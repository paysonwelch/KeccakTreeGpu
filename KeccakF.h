#pragma once

#include "KeccakTypes.h"

#define cKeccakNumberOfRounds 22  // 22

#define ROL32(a, offset) (((a) << (offset)) ^ ((a) >> (32 - offset)))

// implementation of Keccak function on CPU
void KeccakF(tKeccakLane* state);

// implementation of Keccak function on CPU, unrolled
void KeccakF_CPU(tKeccakLane* state);

// set the state to zero
void zeroize(tKeccakLane* state);

// Keccak final node hashing results of previous nodes in sequential mode
// inBuffer supposed to have block_number * output_block_size of data
void Keccak_top(tKeccakLane* Kstate, tKeccakLane* inBuffer, int block_number);

// test equility of 2 keccak states
int isEqual_KS(tKeccakLane* Ks1, tKeccakLane* Ks2);

// print functions
void print_KS(tKeccakLane* state);
void print_KS_256(tKeccakLane* state);

