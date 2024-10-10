#pragma once

#include <vector>

#include "Position.h"

namespace Checkers
{

class Simulator
{
public:
	Simulator() = default;
	virtual ~Simulator() = default;

	virtual void Simulate(const std::vector<Position> &positions, std::vector<int> &blackInc, std::vector<int> &whiteInc, std::vector<int> &visitsInc) = 0;

	static Simulator *CreateHost();
	static Simulator *CreateDevice(unsigned int blockCount, unsigned int threadsPerBlock);
};

}
