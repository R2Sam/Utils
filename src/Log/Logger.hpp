#pragma once

#include "Log/Log.hpp"
#include "Types.hpp"
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

class Logger
{
public:

	struct Color
	{
		u32 r = 0;
		u32 g = 0;
		u32 b = 0;
	};

	inline static constexpr Color RED = {230, 41, 55};
	inline static constexpr Color GREEN = {0, 228, 48};
	inline static constexpr Color YELLOW = {253, 249, 0};
	inline static constexpr Color BLUE = {0, 150, 255};
	inline static constexpr Color CYAN = {0, 255, 255};
	inline static constexpr Color MAGENTA = {255, 0, 255};
	inline static constexpr Color WHITE = {200, 200, 200};
	inline static constexpr Color GRAY = {150, 150, 150};

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

		std::string prefix = std::format("[{}] [{}] ", GetCurrentTimeString(), LevelName(levelIn));

		if (s_file.is_open())
		{
			std::print(s_file, "{}", prefix);
			(std::print(s_file, "{}", CheckOperator(std::forward<Args>(args))), ...);
			std::println(s_file, "");
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
		std::lock_guard<std::mutex> lock(s_mutex);

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
		std::lock_guard<std::mutex> lock(s_mutex);

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

	inline static std::ofstream s_file;
	inline static LogLevel s_level = LogLevel::DEBUG;
	inline static std::mutex s_mutex;

	inline static std::vector<Color> s_levelColors = {{200, 200, 200}, {0, 255, 255}, {0, 228, 48}, {253, 249, 0},
	{230, 41, 55}, {255, 0, 255}};
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