#include <cuda.h>
#include <cuda_runtime.h>

#include "GraphicsCardConfig.h"

namespace Checkers
{

int GetSmCount()
{
	cudaDeviceProp prop;
	cudaGetDeviceProperties(&prop, 0);

	return prop.multiProcessorCount;
}

int GetThreadsPerSm()
{
    cudaDeviceProp prop;
    cudaGetDeviceProperties(&prop, 0);

    switch (prop.major)
    {
    case 6:
        return prop.minor == 0 ? 64 : 128;
    case 7:
        return 64;
    case 8:
        return prop.minor == 0 ? 64 : 128;
    case 9:
        return 128;
    default:
        return 64;
    }

}

}