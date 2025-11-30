#include <iostream>
#include <windows.h>
#undef max // undefine max macro to avoid conflicts with std::numeric_limits

#include "core.hpp"


namespace OWC
{
	template<LogLevel level>
	inline void Log()
	{
		using enum LogLevel;
		if constexpr (level == NewLine)
			std::cout << '\n';
		else
			static_assert(false, "Log function called without message format string!");
	}

	template<LogLevel level, typename... Args>
	inline void Log(const std::format_string<Args...> str, Args&&... args)
	{
		using enum LogLevel;
		if constexpr (level == Trace)
		{
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 8);
			std::cout << "[Trace]: " << std::format(str, std::forward<Args>(args)...) << '\n';
		}
		else if constexpr (level == Debug)
		{
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 232);
			std::cout << "[Debug]: " << std::format(str, std::forward<Args>(args)...);

			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 8);
			std::cout << '\n';
		}
		else if constexpr (level == Warn)
		{
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 14);
			std::cout << "[Warning]: " << std::format(str, std::forward<Args>(args)...);

			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 8);
			std::cout << '\n';
		}
		else if constexpr (level == Error)
		{
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 13);
			std::cout << "[Error]: " << std::format(str, std::forward<Args>(args)...);

			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 8);
			std::cout << '\n';
		}
		else if constexpr (level == Critical)
		{
			if constexpr (!IsDistributionMode())
			{
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 192);
				std::cout << "[Critical]: " << std::format(str, std::forward<Args>(args)...);

				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 8);
				std::cout << '\n';
				__debugbreak();
			}
			else
				MessageBoxA(nullptr, std::format(str, std::forward<Args>(args)...).c_str(), "Critical Error", MB_OK | MB_ICONERROR);
			
			exit(EXIT_FAILURE);
		}
		else // compile-time safeguard
		{
			static_assert(false, "Unknown LogLevel!");
		}
	}
}
