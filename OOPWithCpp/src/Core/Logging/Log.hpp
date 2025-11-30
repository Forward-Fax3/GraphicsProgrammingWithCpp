#pragma once
#include <format>


namespace OWC
{
	enum class LogLevel : uint8_t
	{
		NewLine,
		Trace,
		Debug,
		Warn,
		Error,
		Critical
	};

	template<LogLevel level, typename... Args>
	void Log(const std::format_string<Args...> str, Args&&... args);
}

#include "Log.inl"
