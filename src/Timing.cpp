#include "Timing.hpp"

Stopwatch::Stopwatch()
{
}

Stopwatch::Stopwatch(const std::string& name)
{
	m_name = name;
	m_start = std::chrono::steady_clock::now();
}

Stopwatch::~Stopwatch()
{
	if (!m_done)
	{
		Stop();
	}
}

void Stopwatch::Start()
{
	Start("");
}

void Stopwatch::Start(const std::string& name)
{
	Stop();

	m_name = name;
	m_start = std::chrono::steady_clock::now();

	m_done = false;
}

double Stopwatch::Stop()
{
	return Stop("");
}

double Stopwatch::Stop(const std::string& text)
{
	if (m_done)
	{
		return 0.0;
	}

	m_end = std::chrono::steady_clock::now();
	m_done = true;

	double microseconds = std::chrono::duration_cast<std::chrono::microseconds>(m_end - m_start).count();
	if (!m_name.empty())
	{
		Log(m_name, " took: ", microseconds * 0.001, " ms ", text);
	}

	return microseconds * 0.001;
}

Timer::Timer(const u64 timeMs, const std::function<void()>& callback) :
m_timeMs(timeMs),
m_callback(callback)
{
	m_start = std::chrono::steady_clock::now();
}

Timer::~Timer()
{
	i64 differance =
	std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - m_start).count() -
	m_timeMs;
	if (differance >= 0)
	{
		if (m_callback)
		{
			m_callback();
		}
	}
}

bool Timer::Check()
{
	i64 differance =
	std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - m_start).count() -
	m_timeMs;
	if (differance >= 0)
	{
		if (m_callback)
		{
			m_callback();
		}

		return true;
	}

	return false;
}