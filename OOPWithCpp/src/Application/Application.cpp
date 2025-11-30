#include "Application.hpp"
#include "BaseEvent.hpp"
#include "WindowCloseEvent.hpp"
#include "WindowResize.hpp"
#include "Renderer.hpp"
#include "Log.hpp"
#include "TestLayer.hpp"


namespace OWC
{
	Application* Application::s_Instance = nullptr;

	Application::Application(std::bitset<2>& runFlags)
		: m_RunFlags(runFlags)
	{
		if (!s_Instance)
			s_Instance = this;
		else
			Log<LogLevel::Critical>("Application instance already exists!");

		WindowProperties props {
			.Title = u8"OOP With Cpp",
			.Width = 1920,
			.Height = 1080
		};

		m_Window = std::make_unique<Window>(props);
		m_Window->SetEventCallback([](BaseEvent& e) { s_Instance->OnEvent(e); });
		Renderer::Init();
		m_LayerStack = std::make_unique<LayerStack>();
		PushLayer(std::make_shared<TestLayer>());
	}

	Application::~Application()
	{
		m_Shader.reset();
		m_LayerStack->ClearLayers();
		m_LayerStack.reset();
		Renderer::Shutdown();
		m_Window.reset();
	}

	void Application::Run() const
	{
		while (m_RunFlags.test(0)) // While application is running
		{
			m_LayerStack->OnUpdate();
			m_LayerStack->ImGuiRender();

			Renderer::FinishRender();
			m_Window->Update();
		}
	}

	void Application::OnEvent(BaseEvent& event) const
	{
		EventDispatcher dispacher(event);

		dispacher.Dispatch<WindowCloseEvent>([](const WindowCloseEvent&) {
			s_Instance->Stop();
			return true;
			});

		if (event.HasBeenHandled())
			return;

		dispacher.Dispatch<WindowResize>([](const WindowResize& e) {
			return s_Instance->m_Window->Resize(e.GetWidth(), e.GetHeight());
			});

		if (event.HasBeenHandled())
			return;

		m_LayerStack->OnEvent(event);
	}
} // namespace OWC	