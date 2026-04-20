#pragma once

#include "Log/Log.hpp"
#include "Types.hpp"
#include <fstream>
#include <mutex>
#include <source_location>
#include <stacktrace>
#include <vector>

/**
 * @brief Console and file logging levels
 */

enum class LogLevel : u8
{
	TRACE,
	DEBUG,
	INFO,
	WARN,
	ERROR,
	FATAL
};

/**
 * @brief Thread safe logging utility with differing levels for both console and file
 */

class Logger
{
public:

	struct Color
	{
		u32 r = 0;
		u32 g = 0;
		u32 b = 0;
	};

	/**
	 * @brief Sets minimum log level
	 *
	 * @param levelIn Minimum log level to output
	 */

	static void SetLogLevel(const LogLevel levelIn);

	/**
	 * @brief Sets the output file
	 *
	 * If a file is already opened it is closed.
	 *
	 * @param path Path to log file, empty string disables file output
	 */

	static void SetLogFile(const char* path);

	/**
	 * @brief Writes a log message
	 *
	 * Outputs a message to stderr and a file if set,
	 * depending if the log level is higher than the set minimum.
	 *
	 * @tparam levelIn Log level which to write with
	 * @tparam Args Strings and other printable
	 * @param args Values to be printed separated by commas
	 *
	 * Usage:
	 * @code
	 * logger.Write<LogLevel::INFO>("Value is: ", number);
	 * @endcode
	 */

	template <LogLevel levelIn = LogLevel::DEBUG, typename... Args>
	static void Write(Args&&... args)
	{
		if (levelIn < s_level)
		{
			return;
		}

#ifdef NDEBUG
		if (levelIn < LogLevel::INFO)
		{
			return;
		}
#endif

		std::lock_guard<std::mutex> lock(s_mutex);

		if (s_file.is_open())
		{
			s_file << "[" << GetCurrentTimeString() << "] " << "[" << LevelName(levelIn) << "] ";
			(CheckOperator(s_file, args), ...);
			s_file << '\n';
		}

		std::cerr << LevelColor(levelIn) << "[" << GetCurrentTimeString() << "] " << "[" << LevelName(levelIn) << "] ";
		(CheckOperator(std::cerr, args), ...);
		std::cerr << ANSI_RESET << '\n';
	}

	/**
	 * @brief Allows you to set output text color for a certain log level
	 *
	 * @param level Log level who's color to change
	 * @param color Text output color in rgb
	 */

	static void SetLogLevelColor(const LogLevel level, const Color color);

private:

	static const char* LevelName(const LogLevel level);

	static std::string LevelColor(const LogLevel level);

	static std::string RgbToAnsi(const Color color);

	inline static std::ofstream s_file;
	inline static LogLevel s_level = LogLevel::DEBUG;
	inline static std::mutex s_mutex;

	inline static std::vector<Color> s_levelColors = {{200, 200, 200}, {0, 255, 255}, {0, 228, 48}, {253, 249, 0},
	{230, 41, 55}, {255, 0, 255}};
};

template <LogLevel level = LogLevel::TRACE, bool stack = false>
void TraceFunction(const std::string& message = "",
const std::source_location location = std::source_location::current())
{
	if constexpr (stack)
	{
		const auto trace = std::stacktrace::current();
		std::string traceText;

		for (u32 i = 1; i < trace.size(); ++i)
		{
			const auto line = trace[i];
			traceText +=
			line.description() + " at: " + line.source_file() + ":" + std::to_string(line.source_line()) + '\n';
		}

		Logger::Write<level>(message, '\n', traceText);
	}

	else
	{
		Logger::Write<level>(location.function_name(), " at: ", location.file_name(), "\n", message);
	}
}

template <typename Object, LogLevel level = LogLevel::TRACE>
void TraceValue(const Object& object, const char* objectName)
{
	Logger::Write<level>(Demangle(typeid(object)), "\n", objectName, " value: ", object);
}

template <typename Object, LogLevel level = LogLevel::TRACE>
void TraceAddress(const Object& object, const char* objectName)
{
	Logger::Write<level>(Demangle(typeid(object)), "\n", objectName, " address: ", &object);
}