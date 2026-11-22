#include "Logger.hpp"

// Single definitions live here so every module shares one Logger state.
std::ofstream Logger::s_file;
LogLevel Logger::s_level = LogLevel::DEBUG;
std::mutex Logger::s_mutex;
std::vector<Logger::Color> Logger::s_levelColors = {{.r = 200, .g = 200, .b = 200}, {.r = 0, .g = 255, .b = 255},
{.r = 0, .g = 228, .b = 48}, {.r = 253, .g = 249, .b = 0}, {.r = 230, .g = 41, .b = 55}, {.r = 255, .g = 0, .b = 255}};

void Logger::SetLogLevel(const LogLevel levelIn)
{
	std::scoped_lock lock(s_mutex);

	s_level = levelIn;
}

void Logger::SetLogFile(const char* path)
{
	std::scoped_lock lock(s_mutex);

	if (s_file.is_open())
	{
		s_file.close();
	}

	if (std::string(path).empty())
	{
		return;
	}

	s_file.open(path, std::ios::app);
}

void Logger::SetLogLevelColor(const LogLevel level, const Color color)
{
	u8 index = static_cast<u8>(level);
	if (index > s_levelColors.size())
	{
		return;
	}

	s_levelColors[index] = color;
}

const char* Logger::LevelName(const LogLevel level)
{
	switch (level)
	{
	case LogLevel::TRACE:
		return "TRACE";
	case LogLevel::DEBUG:
		return "DEBUG";
	case LogLevel::INFO:
		return "INFO";
	case LogLevel::WARN:
		return "WARN";
	case LogLevel::ERROR:
		return "ERROR";
	case LogLevel::FATAL:
		return "FATAL";
	}

	return "UNKNOWN";
}

std::string Logger::LevelColor(const LogLevel level)
{
	u8 index = static_cast<u8>(level);
	if (index > s_levelColors.size())
	{
		return LOG_WHITE;
	}

	return RgbToAnsi(s_levelColors[index]);
}

std::string Logger::RgbToAnsi(const Color color)
{
	return std::format("\033[38;2;{};{};{}m", color.r, color.g, color.b);
}