#include <cuda.h>
#include <cuda_runtime.h>
#include <device_launch_parameters.h>

#include <thrust/reduce.h>

#include <stdio.h>

#include <iostream>
#include <random>

#include "Core/Core.h"

#include "DeviceSimulator.h"

namespace Checkers
{

using CudaAssert = Assert<cudaError_t, cudaGetLastError, cudaSuccess, cudaGetErrorString>;

__device__ DeviceGenerator::DeviceGenerator(unsigned int seed, unsigned int scramble)
{
	curand_init(seed, scramble, 0, &m_State);
}

__device__ unsigned int DeviceGenerator::GetUniform(unsigned int low, unsigned int high)
{
	return low + (1.0f - curand_uniform(&m_State)) * high;
}

static __global__ void SetupKernel(unsigned int seed, DeviceGenerator *generators)
{
	int tid = threadIdx.x + blockDim.x * blockIdx.x;

	// Placement new - the memory is already allocated
	// we just want to run the constructor to initialize the object in place
	new (&generators[tid]) DeviceGenerator(seed, tid);
}

DeviceSimulator::DeviceSimulator(unsigned int blockCount, unsigned int threadsPerBlock)
	: m_BlockCount(blockCount), m_ThreadsPerBlock(threadsPerBlock), m_ThreadCount(m_BlockCount* m_ThreadsPerBlock)
{
	std::random_device dev;
	unsigned int seed = dev();

	cudaMalloc(&m_dPositions, sizeof(Position) * m_BlockCount);
	cudaMalloc(&m_dBlackInc, sizeof(int) * m_BlockCount);
	cudaMalloc(&m_dWhiteInc, sizeof(int) * m_BlockCount);

	cudaMalloc(&m_Generators, sizeof(DeviceGenerator) * m_ThreadCount);
	SetupKernel<<<m_BlockCount, m_ThreadsPerBlock>>>(seed, m_Generators);
	cudaDeviceSynchronize();
}

DeviceSimulator::~DeviceSimulator()
{
	// ShutdownKernel invoking the destructors of generators should go here
	cudaFree(m_Generators);

	cudaFree(&m_dPositions);
	cudaFree(&m_dBlackInc);
	cudaFree(&m_dWhiteInc);
}

static __global__ void SimulateKernel(Position *positions, DeviceGenerator *generators, int *blackInc, int *whiteInc)
{
	int tid = threadIdx.x + blockDim.x * blockIdx.x;

	static constexpr int MaxBlockSize = 1024;
	__shared__ int biSum[MaxBlockSize];
	__shared__ int wiSum[MaxBlockSize];

	if (threadIdx.x == 0)
	{
		biSum[blockIdx.x] = 0;
		wiSum[blockIdx.x] = 0;
	}

	Position position = positions[blockIdx.x];
	position.SimulateOne(generators[tid], biSum[threadIdx.x], wiSum[threadIdx.x]);
	__syncthreads();

#pragma unroll
	for (int i = 1; i <= 10; i++)
	{
		const int activeThreads = (blockDim.x >> i);
		if (threadIdx.x < activeThreads)
		{
			biSum[threadIdx.x] += biSum[activeThreads + threadIdx.x];
			wiSum[threadIdx.x] += wiSum[activeThreads + threadIdx.x];
		}
		__syncthreads();
	}

	if (threadIdx.x == 0)
	{
		blackInc[blockIdx.x] = biSum[0];
		whiteInc[blockIdx.x] = wiSum[0];
	}
}

void DeviceSimulator::Simulate(const std::vector<Position> &positions, std::vector<int> &blackInc, std::vector<int> &whiteInc, std::vector<int> &visitsInc)
{
	int blockCount = positions.size();
	if (blockCount > m_BlockCount)
		blockCount = m_BlockCount;

	cudaMemcpy(m_dPositions, positions.data(), sizeof(Position) * blockCount, cudaMemcpyHostToDevice);

	{
		Timer timer("Kernel");
		SimulateKernel<<<blockCount, m_ThreadsPerBlock>>>(m_dPositions, m_Generators, m_dBlackInc, m_dWhiteInc);
		cudaDeviceSynchronize();
	}

	cudaMemcpy(blackInc.data(), m_dBlackInc, sizeof(int) * blockCount, cudaMemcpyDeviceToHost);
	cudaMemcpy(whiteInc.data(), m_dWhiteInc, sizeof(int) * blockCount, cudaMemcpyDeviceToHost);
	std::fill(visitsInc.begin(), visitsInc.begin() + blockCount, m_ThreadsPerBlock * 2);

	std::fill(blackInc.begin() + blockCount, blackInc.end(), 0);
	std::fill(whiteInc.begin() + blockCount, whiteInc.end(), 0);
	std::fill(visitsInc.begin() + blockCount, visitsInc.end(), 0);
}

}
