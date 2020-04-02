#pragma once

#include "KeccakTree.h"
#include "KeccakTypes.h"


// Implement Tree hash mode 1 on CPU
// data to be hashed is present in inBuffer
// output result is in outBuffer
void KeccakTreeCPU(tKeccakLane* inBuffer, tKeccakLane* outBuffer);

void KeccakTreeCPU_2stg(tKeccakLane* inBuffer, tKeccakLane* outBuffer);

