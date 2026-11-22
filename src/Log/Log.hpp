#pragma once
#include <algorithm>
#include <cxxabi.h>
#include <iostream>
#include <mutex>
#include <ostream>
#include <typeindex>

#include "rfl.hpp"
#include "rfl/json.hpp"
#include "utils_export.h"
UTILS_EXPORT extern std::mutex s_logMutex;

UTILS_EXPORT void GetThreadSafeLocalTime(const time_t& timeInput, std::tm& timeInfo);

UTILS_EXPORT std::string GetCurrentTimeString();

inline const char* const ANSI_RESET = "\033[0m";
inline const char* const ANSI_SAVE_CURSOR = "\033[s";
inline const char* const ANSI_LOAD_CURSOR = "\033[u";
inline const char* const ANSI_HIDE_CURSOR = "\033[?25l";
inline const char* const ANSI_SHOW_CURSOR = "\033[?25h";
inline const char* const ANSI_CLEAR_LINE = "\033[2K";
inline const char* const ANSI_SCROLL_DOWN = "\033[D";

inline const char* const LOG_GREEN = "\033[92m";
inline const char* const LOG_BLUE = "\033[94m";
inline const char* const LOG_YELLOW = "\033[93m";
inline const char* const LOG_RED = "\033[91m";
inline const char* const LOG_WHITE = "\033[97m";

template <typename T>
concept HasOstreamOperator = requires(std::ostream& os, const T& value) // NOLINT
{
	{ os << value } -> std::same_as<std::ostream&>;
};

template <typename T>
concept HasPrintFormatter = requires(const T& value) {
	{ std::format("{}", value) } -> std::same_as<std::string>;
};

template <typename T>
concept HasStringOperator = requires(const T& value) // NOLINT
{
	{ static_cast<std::string>(value) } -> std::same_as<std::string>;
};

template <typename T>
std::string CheckOperator(const T& object)
{
	if constexpr (HasPrintFormatter<T>)
	{
		return std::format("{}", object);
	}

	else if constexpr (HasStringOperator<T>)
	{
		return static_cast<std::string>(object);
	}

	return "[INVALID]";
}

template <typename T>
void CheckOperator(std::ostream& os, const T& object)
{
	if constexpr (HasOstreamOperator<T>)
	{
		os << object;
		return;
	}

	else if constexpr (HasStringOperator<T>)
	{
		os << static_cast<std::string>(object);
		return;
	}

	os << "[INVALID]";
}

template <typename T>
std::string CleanJson(const T& object)
{
	std::string json = rfl::json::write<rfl::SnakeCaseToCamelCase>(object);
	std::erase(json, '{');
	std::erase(json, '}');
	std::erase(json, '"');

	size_t pos = 0;
	while ((pos = json.find(':', pos)) != std::string::npos)
	{
		if (pos + 1 < json.size() && json[pos + 1] != ' ')
		{
			json.insert(pos + 1, " ");
			pos += 2;
		}

		else
		{
			pos++;
		}
	}

	pos = 0;
	while ((pos = json.find(',', pos)) != std::string::npos)
	{
		if (pos + 1 < json.size() && json[pos + 1] != ' ')
		{
			json.insert(pos + 1, " ");
			pos += 2;
		}

		else
		{
			pos++;
		}
	}

	return json;
}

template <typename... Args>
[[deprecated("Use Logger::Write() instead")]]
void Log(Args&&... args)
{
	std::scoped_lock lock(s_logMutex);

#ifdef LOGFILE
	std::ofstream logFile("log.txt", std::ios_base::app);
	logFile << "[" << GetCurrentTimeString() << "] ";
	(CheckOperator(logFile, args), ...);
	logFile << '\n';
#endif

	std::cerr << LOG_WHITE << "[" << GetCurrentTimeString() << "] ";
	(CheckOperator(std::cerr, args), ...);
	std::cerr << ANSI_RESET << '\n';
}

template <typename... Args>
[[deprecated("Use Logger::Write() instead")]]
void LogColor(const char* color, Args&&... args)
{
	std::scoped_lock lock(s_logMutex);

#ifdef LOGFILE
	std::ofstream logFile("log.txt", std::ios_base::app);
	logFile << "[" << GetCurrentTimeString() << "] ";
	(CheckOperator(logFile, args), ...);
	logFile << '\n';
#endif

	std::cerr << LOG_WHITE << "[" << GetCurrentTimeString() << "] ";
	std::cerr << color;
	(CheckOperator(std::cerr, args), ...);
	std::cerr << ANSI_RESET << '\n';
}

template <typename... Args>
[[deprecated("Use Logger::Write() instead")]]
void Output(Args&&... args)
{
	std::scoped_lock lock(s_logMutex);

	(CheckOperator(std::cout, args), ...);
	std::cout << ANSI_RESET << '\n';
}

template <typename... Args>
[[deprecated("Use Logger::Write() instead")]]
void OutputColor(const char* color, Args&&... args)
{
	std::scoped_lock lock(s_logMutex);

	std::cout << color;
	(CheckOperator(std::cout, args), ...);
	std::cout << ANSI_RESET << '\n';
}

template <typename... Args>
[[deprecated("Use Logger::Write() instead")]]
void OutputErr(Args&&... args)
{
	std::scoped_lock lock(s_logMutex);

	(CheckOperator(std::cerr, args), ...);
	std::cerr << ANSI_RESET << '\n';
}

template <typename... Args>
[[deprecated("Use Logger::Write() instead")]]
void OutputErrColor(const char* color, Args&&... args)
{
	std::scoped_lock lock(s_logMutex);

	std::cerr << color;
	(CheckOperator(std::cerr, args), ...);
	std::cerr << ANSI_RESET << '\n';
}

#ifdef _WIN32

#ifndef _INC_WINDOWS

extern "C"
{
	__declspec(dllimport) void* __stdcall GetStdHandle(unsigned long);
	__declspec(dllimport) int __stdcall GetConsoleMode(void*, unsigned long*);
	__declspec(dllimport) int __stdcall SetConsoleMode(void*, unsigned long);
	__declspec(dllimport) int __stdcall SetConsoleTextAttribute(void*, unsigned short);
}

#endif

class UTILS_EXPORT Ansi
{

public:

	Ansi();

	~Ansi();
};

#endif

template <typename T>
std::string Demangle()
{
	std::string name = typeid(T).name();

	int status = -4;

	std::unique_ptr<char, void (*)(void*)> res{abi::__cxa_demangle(name.c_str(), nullptr, nullptr, &status), std::free};

	return (status == 0) ? res.get() : name;
}

UTILS_EXPORT std::string Demangle(const std::type_index& type);

template <typename T>
std::string DemangleWithoutNamespace()
{
	std::string demangled = Demangle<T>();

	size_t pos = demangled.find_last_of("::");
	if (pos != std::string::npos)
	{
		return demangled.substr(pos + 1);
	}

	return demangled;
}

UTILS_EXPORT std::string DemangleWithoutNamespace(const std::type_index& type);