#pragma once

#include "Log/Log.hpp"
#include "Types.hpp"
#include "utils_export.h"
#include <format>
#include <fstream>
#include <iostream>
#include <mutex>
#include <print>
#include <source_location>
#include <vector>

#ifndef __EMSCRIPTEN__
#include <stacktrace>
#endif

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

class UTILS_EXPORT Logger
{
public:

	struct Color
	{
		u32 r = 0;
		u32 g = 0;
		u32 b = 0;
	};

	static constexpr Color RED = {.r = 230, .g = 41, .b = 55};
	static constexpr Color GREEN = {.r = 0, .g = 228, .b = 48};
	static constexpr Color YELLOW = {.r = 253, .g = 249, .b = 0};
	static constexpr Color BLUE = {.r = 0, .g = 150, .b = 255};
	static constexpr Color CYAN = {.r = 0, .g = 255, .b = 255};
	static constexpr Color MAGENTA = {.r = 255, .g = 0, .b = 255};
	static constexpr Color WHITE = {.r = 200, .g = 200, .b = 200};
	static constexpr Color GRAY = {.r = 150, .g = 150, .b = 150};

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

		std::scoped_lock lock(s_mutex);

		std::string prefix = std::format("[{}] [{}] ", GetCurrentTimeString(), LevelName(levelIn));

		if (s_file.is_open())
		{
			std::print(s_file, "{}", prefix);
			(std::print(s_file, "{}", CheckOperator(std::forward<Args>(args))), ...);
		}

		std::print(std::cerr, "{}{}", LevelColor(levelIn), prefix);
		(std::print(std::cerr, "{}", CheckOperator(std::forward<Args>(args))), ...);
		std::println(std::cerr, "{}", ANSI_RESET);
	}

	/**
	 * @brief Writes a message to stdout without a trailing newline
	 *
	 * Uses the same formatting rules as Write, but prints to stdout,
	 * adds no prefix, no log level, and no trailing newline.
	 *
	 * @tparam Args Strings and other printable
	 * @param args Values to be printed separated by commas
	 */

	template <typename... Args>
	static void Print(Args&&... args)
	{
		std::scoped_lock lock(s_mutex);

		(std::print(std::cout, "{}", CheckOperator(std::forward<Args>(args))), ...);
		std::cout.flush();
	}

	/**
	 * @brief Writes a colored message to stdout without a trailing newline
	 *
	 * Uses the same formatting rules as Print, but wraps the output in
	 * the ANSI escape for the given color and resets afterwards.
	 *
	 * @param color Foreground color to use
	 * @tparam Args Strings and other printable
	 * @param args Values to be printed separated by commas
	 */

	template <typename... Args>
	static void Print(const Color color, Args&&... args)
	{
		std::scoped_lock lock(s_mutex);

		std::print(std::cout, "{}", RgbToAnsi(color));
		(std::print(std::cout, "{}", CheckOperator(std::forward<Args>(args))), ...);
		std::print(std::cout, "{}", ANSI_RESET);
		std::cout.flush();
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

	static std::ofstream s_file;
	static LogLevel s_level;
	static std::mutex s_mutex;

	static std::vector<Color> s_levelColors;
};

template <bool stack = false>
std::string GetTrace(const std::source_location location = std::source_location::current())
{
	if constexpr (stack)
	{
#ifndef __EMSCRIPTEN__
		const auto trace = std::stacktrace::current();
		std::string traceText;

		for (u32 i = 1; i < trace.size(); ++i)
		{
			const auto line = trace[i];
			traceText +=
			line.description() + " at: " + line.source_file() + ":" + std::to_string(line.source_line()) + '\n';
		}

		return traceText;
#endif
	}

	return std::string(location.function_name()) + " at: " + location.file_name() + "\n";
}

template <LogLevel level = LogLevel::TRACE, bool stack = false>
void TraceFunction(const std::string& message = "",
const std::source_location location = std::source_location::current())
{
	if constexpr (stack)
	{
#ifndef __EMSCRIPTEN__
		Logger::Write<level>(message, '\n', GetTrace<stack>());
		return;
#else
		Logger::Write<level>("Cannot do stack trace in emscripten");
#endif
	}

	Logger::Write<level>(GetTrace<stack>());
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

template <typename... Args>
void PrintToFile(const char* path, std::ios_base::openmode mode, Args&&... args)
{
	std::ofstream file;
	file.open(path, mode);

	if (file.is_open())
	{
		(std::print(file, "{}", CheckOperator(std::forward<Args>(args))), ...);
		file.close();
	}

	else
	{
		Logger::Write<LogLevel::ERROR>("Could not open file to print: ", path);
	}
}

template <typename... Args>
void PrintToFile(std::ofstream& file, Args&&... args)
{
	if (file.is_open())
	{
		(std::print(file, "{}", CheckOperator(std::forward<Args>(args))), ...);
	}

	else
	{
		Logger::Write<LogLevel::ERROR>("File was not open to print to");
	}
}