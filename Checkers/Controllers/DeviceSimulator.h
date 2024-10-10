#pragma once

#include <curand.h>
#include <curand_kernel.h>

#include <thrust/device_vector.h>

#include "Simulator.h"

namespace Checkers
{

class DeviceGenerator
{
public:
	__device__ DeviceGenerator(unsigned int seed, unsigned int offset);

	__device__ unsigned int GetUniform(unsigned int low, unsigned int high);

private:
	curandState_t m_State;
};

class DeviceSimulator : public Simulator
{
public:
	DeviceSimulator(unsigned int blockCount, unsigned int threadsPerBlock);
	~DeviceSimulator() override;

	void Simulate(const std::vector<Position> &positions, std::vector<int> &blackInc, std::vector<int> &whiteInc, std::vector<int> &visitsInc) override;

private:
	unsigned int m_BlockCount, m_ThreadsPerBlock, m_ThreadCount;

	DeviceGenerator *m_Generators;

	Position *m_dPositions;
	int *m_dBlackInc, *m_dWhiteInc;
};

}
