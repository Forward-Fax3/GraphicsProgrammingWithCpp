#include "Application.hpp"
#include "BaseEvent.hpp"
#include "WindowCloseEvent.hpp"
#include "WindowResize.hpp"

#include <iostream>


namespace OWC
{
	Application::Application(std::bitset<2>& runFlags)
		: m_RunFlags(runFlags)
	{
		WindowProperties props {
			.Title = u8"OOP With Cpp",
			.Width = 1920,
			.Height = 1080
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

		dispacher.Dispatch<WindowResize>([this](const WindowResize& e) {
			return this->m_Window->Resize(e.GetWidth(), e.GetHeight());
			});
	}

	void Application::OnWindowClose()
	{
		Stop();
	}
} // namespace OWC	