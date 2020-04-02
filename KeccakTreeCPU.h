#pragma once

#include "KeccakTree.h"

// Implement Tree hash mode 1 on CPU
// data to be hashed is present in inBuffer
// output result is in outBuffer
void KeccakTreeCPU(unsigned* inBuffer, unsigned* outBuffer);

void KeccakTreeCPU_2stg(unsigned* inBuffer, unsigned* outBuffer);
