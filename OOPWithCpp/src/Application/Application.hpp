#pragma once
#include <memory>
#include <bitset>

#include "Window.hpp"
#include "BaseEvent.hpp"


// OWC - OOP With Cpp
namespace OWC
{
	class Application
	{
	public:
		Application() = delete;
		explicit Application(std::bitset<2>& runFlags);
		~Application() = default;

		void Run();

		void Stop() { m_RunFlags.set(0, false); } // set run flag to shutdown
		void Restart() { Stop(); m_RunFlags.set(1, true); } // Set shutdown and restart flags

	private:
		void OnEvent(BaseEvent& event);
		void OnWindowClose();

	private:
		std::bitset<2>& m_RunFlags; // Bit 0: Application running, Bit 1: restart application

		std::unique_ptr<Window> m_Window = nullptr;
	};
}
