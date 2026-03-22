#pragma once

#include "Log/Log.hpp"

#include "Types.hpp"

#include <chrono>
#include <deque>
#include <functional>

class Stopwatch
{
public:

	Stopwatch();
	Stopwatch(const std::string& name);

	~Stopwatch();

	void Start();
	void Start(const std::string& name);

	double Stop();
	double Stop(const std::string& text);

private:

	std::chrono::time_point<std::chrono::steady_clock> m_start;
	std::chrono::time_point<std::chrono::steady_clock> m_end;

	bool m_done = true;

	std::string m_name;
};

class Timer
{
public:

	Timer(const u64 timeMs, const std::function<void()>& callback = nullptr);
	~Timer();

	bool Check();

private:

	u64 m_timeMs = 0;
	std::chrono::time_point<std::chrono::steady_clock> m_start;
	std::function<void()> m_callback;
};

template <typename T>
class RollingAverage
{
public:

	RollingAverage(const u64 samples = 100)
	{
		m_sampleCount = samples;
	}

	void Add(const T value)
	{
		m_samples.push_back(value);

		if (m_samples.size() > m_sampleCount)
		{
			m_samples.pop_front();
		}
	}

	void operator+=(const T value)
	{
		Add(value);
	}

	T Average() const
	{
		if (m_samples.empty())
		{
			return 0;
		}

		T sum = 0;

		for (const float value : m_samples)
		{
			sum += value;
		}

		return sum / m_samples.size();
	}

private:

	std::deque<T> m_samples;
	u64 m_sampleCount;
};