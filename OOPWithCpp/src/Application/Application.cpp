#include "Application.hpp"
#include "Renderer.hpp"

#include "BaseEvent.hpp"
#include "WindowCloseEvent.hpp"
#include "WindowResize.hpp"
#include "WindowMinimizeEvent.hpp"
#include "WindowRestoreEvent.hpp"

#include "Log.hpp"

#include "TestLayer.hpp"
#include "ImGuiLayer.hpp"


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
		Graphics::Renderer::Init();
		m_LayerStack = std::make_unique<LayerStack>();
		PushLayer(std::make_shared<TestLayer>());
		m_ImGuiLayer = std::make_shared<ImGuiLayer>();
		PushOverlay(m_ImGuiLayer);
	}

	Application::~Application()
	{
		m_Shader.reset();
		m_LayerStack->ClearLayers();
		m_LayerStack.reset();
		m_ImGuiLayer.reset(); // ImGuiLayer must be destroyed before Renderer shutdown
		Graphics::Renderer::Shutdown();
		m_Window.reset();
	}

	void Application::Run() const
	{
		while (m_RunFlags.test(0)) // While application is running
		{
			m_LayerStack->OnUpdate();

			m_ImGuiLayer->Begin();
			m_LayerStack->ImGuiRender();
			m_ImGuiLayer->End();

			Graphics::Renderer::FinishRender();
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

		dispacher.Dispatch<WindowResize>([](const WindowResize& e) {
			return s_Instance->m_Window->Resize(e.GetWidth(), e.GetHeight());
			});

		dispacher.Dispatch<WindowMinimize>([](const WindowMinimize&) {
			s_Instance->m_Window->Minimize();
			return false; // layers may need to use the event as well
			});

		dispacher.Dispatch<WindowRestore>([](const WindowRestore&) {
			s_Instance->m_Window->Restore();
			return false; // layers may need to use the event as well
			});

		if (event.HasBeenHandled())
			return;

		m_LayerStack->OnEvent(event);
	}
} // namespace OWC	