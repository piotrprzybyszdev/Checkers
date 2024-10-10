#include "DeviceSimulator.h"
#include "HostSimulator.h"
#include "Simulator.h"

namespace Checkers
{

Simulator *Simulator::CreateHost()
{
	return new HostSimulator();
}

Simulator *Simulator::CreateDevice(unsigned int blockCount, unsigned int threadsPerBlock)
{
	return new DeviceSimulator(blockCount, threadsPerBlock);
}

}