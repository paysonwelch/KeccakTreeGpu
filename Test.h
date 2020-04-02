#pragma once

//********************
// Basic treehash mode
//********************

// test Tree hash mode 1 in CPU only
// integer in argument is to lower the workload for CPU only test (slower than
// GPU tests)
void TestCPU(unsigned);

// Test Tree hash mode 1 with GPU and CPU
void TestGPU(unsigned);

// Test Tree hash mode 1 , GPU and CPU, CPU computation overlapped with GPU
// computation
void TestGPU_OverlapCPU(unsigned);


void TestGPU_Split(unsigned);

// Test Tree hash mode 1 , GPU and CPU, GPU computation is overlapped with
// memory transfers (Host to device)
void TestGPU_Stream(unsigned);

// Test Tree hash mode 1 , GPU and CPU, GPU computation is overlapped with
// memory transfers , and with CPU computation
void TestGPU_Stream_OverlapCPU(unsigned);

// use of mapped memory : untested, unsupported by authors hardware
void TestGPU_MappedMemory(unsigned);

//*************
// 2 stages hash
//*************
void TestCPU_2stg(unsigned);

void TestGPU_2stg(unsigned);

void TestGPU_2stg_Stream_OverlapCPU(unsigned);


//***************************
// Keccak in StreamCipher mode
//***************************
void TestGPU_SCipher(unsigned);


// Other function

// Empirically Test if all words in input data are taken into the hash function
void Test_Completness(void);


// print GPU device info
void Device_Info(void);

// print Tree hash mode params set in KeccakTree.h
void Print_Param(unsigned);
