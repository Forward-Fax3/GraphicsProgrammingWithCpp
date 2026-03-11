#include <bitset>
#include <iostream>

#include "Application.hpp"


#if defined(_WIN32)
	#define ENTRY_POINT_EXPORT __declspec(dllexport)
#elif defined(__GNUC__)
	#define ENTRY_POINT_EXPORT __attribute__((visibility("default")))
#else
	#define ENTRY_POINT_EXPORT
#endif

extern "C" ENTRY_POINT_EXPORT void EntryPoint(int, char**)
{
	std::bitset<2> runFlags{ 0b10 }; // Bit 0: Application running, Bit 1: restart application

	while (runFlags.test(1))
	{
		runFlags.set(0, true);  // Set application running flag
		runFlags.set(1, false); // Reset restart flag

		OWC::Application app(runFlags);
		app.Run();
	}
}
