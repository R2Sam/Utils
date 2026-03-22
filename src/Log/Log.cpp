#include "Log.hpp"

static std::mutex s_timeMutex;

void GetThreadSafeLocalTime(const time_t& timeInput, std::tm& timeInfo)
{
	std::lock_guard<std::mutex> lock(s_timeMutex);
	std::tm* result = localtime(&timeInput);
	if (result != nullptr)
	{
		timeInfo = *result;
	}
}

std::string GetCurrentTimeString()
{
	auto now = std::chrono::system_clock::now();
	auto inTimeT = std::chrono::system_clock::to_time_t(now);
	auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

	std::tm timeInfo;
	std::stringstream ss;

	GetThreadSafeLocalTime(inTimeT, timeInfo);

	char buffer[9];
	strftime(buffer, sizeof(buffer), "%H:%M:%S", &timeInfo);

	ss << buffer << '.' << std::setw(3) << std::setfill('0') << static_cast<int>(milliseconds.count());

	return ss.str();
}

std::string Demangle(const std::type_index& type)
{
	const char* name = type.name();

	int status = -4;

	std::unique_ptr<char, void (*)(void*)> res{abi::__cxa_demangle(name, nullptr, nullptr, &status), std::free};

	return (status == 0) ? res.get() : name;
}

std::string DemangleWithoutNamespace(const std::type_index& type)
{
	std::string demangled = Demangle(type);

	size_t pos = demangled.find_last_of("::");
	if (pos != std::string::npos)
	{
		return demangled.substr(pos + 1);
	}

	return demangled;
}

#ifdef _WIN32

Ansi::Ansi()
{
	void* hOutput = GetStdHandle(-11);
	unsigned long outMode;
	GetConsoleMode(hOutput, &outMode);
	SetConsoleMode(hOutput, outMode | 0x0004);

	void* hError = GetStdHandle(-12);
	unsigned long errMode;
	GetConsoleMode(hError, &errMode);
	SetConsoleMode(hError, errMode | 0x0004);
}

Ansi::~Ansi()
{
	void* hOutput = GetStdHandle(-11);
	unsigned long outMode;
	GetConsoleMode(hOutput, &outMode);
	SetConsoleMode(hOutput, outMode & ~0x0004);

	void* hError = GetStdHandle(-12);
	unsigned long errMode;
	GetConsoleMode(hError, &errMode);
	SetConsoleMode(hError, errMode & ~0x0004);
}

Ansi ansi;

#endif