#include "Application.hpp"
#include "BaseEvent.hpp"
#include "WindowCloseEvent.hpp"

#include <iostream>


namespace OWC
{
	Application::Application(std::bitset<2>& runFlags)
		: m_RunFlags(runFlags)
	{
		WindowProperties props {
			.Title = L"OOP With Cpp",
			.Width = 1280,
			.Height = 720
		};
		m_Window = std::make_unique<Window>(props);
		m_Window->SetEventCallback([this](BaseEvent& e) { this->OnEvent(e); });
	}

	void Application::Run()
	{
		while (m_RunFlags.test(0)) // While application is running
		{
			m_Window->Update();
		}
	}

	void Application::OnEvent(BaseEvent& event)
	{
		EventDispatcher dispacher(event);

		dispacher.Dispatch<WindowCloseEvent>([this](const WindowCloseEvent&) {
			this->OnWindowClose();
			return true;
		});
	}

	void Application::OnWindowClose()
	{
		Stop();
	}
} // namespace OWC	