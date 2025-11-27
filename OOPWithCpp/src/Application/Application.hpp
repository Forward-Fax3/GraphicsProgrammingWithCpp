#pragma once
#include <memory>
#include <bitset>

#include "Window.hpp"
#include "BaseEvent.hpp"
#include "LayerStack.hpp"


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
		std::unique_ptr<Window> m_Window = nullptr;
		std::unique_ptr<LayerStack> m_LayerStack = nullptr;
		std::bitset<2>& m_RunFlags; // Bit 0: Application running, Bit 1: restart application
	};
}
