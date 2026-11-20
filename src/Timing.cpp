#include "Timing.hpp"

#include "Log/Logger.hpp"

Stopwatch::Stopwatch() = default;

Stopwatch::Stopwatch(const std::string& name)
{
	m_name = name;
	m_start = std::chrono::steady_clock::now();
}

Stopwatch::~Stopwatch() // NOLINT(bugprone-exception-escape)
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

f64 Stopwatch::Stop()
{
	return Stop("");
}

f64 Stopwatch::Stop(const std::string& text)
{
	if (m_done)
	{
		return 0.0;
	}

	m_end = std::chrono::steady_clock::now();
	m_done = true;

	f64 microseconds = static_cast<f64>(std::chrono::duration_cast<std::chrono::microseconds>(m_end - m_start).count());
	if (!m_name.empty())
	{
		Logger::Write(m_name, " took: ", microseconds * 0.001, " ms ", text);
	}

	return microseconds * 0.001;
}

f64 Stopwatch::Check() const
{
	if (m_done)
	{
		return 0;
	}

	auto now = std::chrono::steady_clock::now();
	f64 microseconds = static_cast<f64>(std::chrono::duration_cast<std::chrono::microseconds>(now - m_start).count());

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
	i64 differance = static_cast<i64>(
	std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - m_start).count() -
	m_timeMs);
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
	i64 differance = static_cast<i64>(
	std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - m_start).count() -
	m_timeMs);
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