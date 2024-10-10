#pragma once

#include "Simulator.h"

#include <random>

namespace Checkers
{

class HostGenerator
{
public:
	HostGenerator(unsigned int seed);

	unsigned int GetUniform(unsigned int low, unsigned int high);

private:
	std::mt19937 m_Engine;
};

class HostSimulator : public Simulator
{
public:
	HostSimulator();
	~HostSimulator() override;

	void Simulate(const std::vector<Position> &positions, std::vector<int> &blackInc, std::vector<int> &whiteInc, std::vector<int> &visitsInc) override;

private:
	HostGenerator m_Generator;
};

}
