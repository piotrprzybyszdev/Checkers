#include "Core.h"

#include <iostream>

namespace Checkers
{

std::map<std::string, std::string> Stats::s_Stats = {};
std::map<std::string, std::chrono::nanoseconds> Stats::s_Measurements = {};

void Stats::AddMeasurement(const std::string &timer, std::chrono::nanoseconds measurement)
{
	s_Measurements[timer] += measurement;
}

void Stats::Clear()
{
	s_Stats.clear();
	s_Measurements.clear();
}

void Stats::FlushTimers()
{
	for (const auto& [timer, measurement] : s_Measurements)
	{
		Stats::AddStat(timer, "{}: {:.3f} ms", timer,
			std::chrono::duration_cast<std::chrono::microseconds>(measurement).count() / 1000.0f
		);
	}
	s_Measurements.clear();
}

const std::map<std::string, std::string> &Stats::GetStats()
{
	return s_Stats;
}

Timer::Timer(std::string &&name)
	: m_Name(name), m_Start(std::chrono::high_resolution_clock::now())
{
}

Timer::~Timer()
{
	Stats::AddMeasurement(m_Name, std::chrono::high_resolution_clock::now() - m_Start);
}

void ThrowError(std::source_location location, const char *message)
{
	throw std::exception(
		std::format(
			"Assertion failed at {}({}:{}): {}: {}", location.file_name(),
			location.line(), location.column(), location.function_name(), message
		).c_str()
	);
}

}