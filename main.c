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

#define IMAX \
    1600  // 2400 // 1600 for high speed mesures // iteration for speed mesure
          // loops

int main(int argc, char** argv)
{
    unsigned imax = IMAX;
    if (argc >= 2)
        imax = atoi(argv[1]);
    Device_Info();

    Print_Param(imax);

    TestCPU_2stg(imax);

    TestGPU_2stg(imax);

    TestGPU_2stg_Stream_OverlapCPU(imax);

    TestGPU_SCipher(imax);

    return 0;
}
