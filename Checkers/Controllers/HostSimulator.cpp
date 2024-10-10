#include <bit>
#include <random>

#include "HostSimulator.h"

namespace Checkers
{

HostGenerator::HostGenerator(unsigned int seed) : m_Engine(seed)
{
}

unsigned int HostGenerator::GetUniform(unsigned int low, unsigned int high)
{
	std::uniform_int_distribution<std::mt19937::result_type> dist(low, high);
	return dist(m_Engine);
}

HostSimulator::HostSimulator() : m_Generator(std::random_device()())
{
}

HostSimulator::~HostSimulator()
{
}

void HostSimulator::Simulate(const std::vector<Position> &positions, std::vector<int> &blackInc, std::vector<int> &whiteInc, std::vector<int> &visitsInc)
{
	std::fill(blackInc.begin(), blackInc.end(), 0);
	std::fill(whiteInc.begin(), whiteInc.end(), 0);
	std::fill(visitsInc.begin(), visitsInc.end(), 0);

	Position position = positions[0];
	position.SimulateOne(m_Generator, blackInc[0], whiteInc[0]);
	visitsInc[0] = 2;
}

}