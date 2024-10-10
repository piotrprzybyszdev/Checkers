#pragma once

#include <chrono>
#include <format>
#include <map>
#include <source_location>
#include <string>

namespace Checkers
{

class Stats
{
public:
	template<typename... Args>
	static void AddStat(std::string statName, std::string &&format, Args... args);

	static void AddMeasurement(const std::string &timer, std::chrono::nanoseconds measurement);

	static void Clear();
	static void FlushTimers();

	static const std::map<std::string, std::string> &GetStats();

private:
	static std::map<std::string, std::string> s_Stats;
	static std::map<std::string, std::chrono::nanoseconds> s_Measurements;
};

template<typename... Args>
void Stats::AddStat(std::string statName, std::string &&format, Args... args)
{
	s_Stats[statName] = std::vformat(format, std::make_format_args(args...));
}

class Timer
{
public:
	Timer(std::string &&name);
	~Timer();

private:
	std::string m_Name;

	std::chrono::time_point<std::chrono::high_resolution_clock> m_Start;
};

void ThrowError(std::source_location location, const char *message);

template<typename T>
static inline const char *GetEmptyMessage(T val)
{
	return "";
}

template<typename T, T (*checkError)(void), T success, const char *(*getErrorMessage)(T) = GetEmptyMessage>
struct Assert
{
	Assert(std::source_location location = std::source_location::current())
	{
#ifndef NDEBUG
		T error = checkError();

		if (error != success)
			ThrowError(location, getErrorMessage(error));
#endif
	}
};

}
