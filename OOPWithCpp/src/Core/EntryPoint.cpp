#include <bitset>

#include "Application.hpp"


extern "C" __declspec(dllexport) void EntryPoint(int, char**)
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
