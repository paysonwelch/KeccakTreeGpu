#pragma once

#define cKeccakNumberOfRounds 22  // 22

#define ROL32(a, offset) (((a) << (offset)) ^ ((a) >> (32 - offset)))

// implementation of Keccak function on CPU
void KeccakF(unsigned* state);

// implementation of Keccak function on CPU, unrolled
void KeccakF_CPU(unsigned* state);

// set the state to zero
void zeroize(unsigned* state);

// Keccak final node hashing results of previous nodes in sequential mode
// inBuffer supposed to have block_number * output_block_size of data
void Keccak_top(unsigned* Kstate, unsigned* inBuffer, int block_number);

// test equility of 2 keccak states
int isEqual_KS(unsigned* Ks1, unsigned* Ks2);

// print functions
void print_KS(unsigned* state);
void print_KS_256(unsigned* state);
