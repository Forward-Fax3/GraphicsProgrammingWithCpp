#include "Application.hpp"
#include "Renderer.hpp"

#include "BaseEvent.hpp"
#include "WindowCloseEvent.hpp"
#include "WindowResize.hpp"
#include "WindowMinimizeEvent.hpp"
#include "WindowRestoreEvent.hpp"
#include "KeyEvent.hpp"

#include <SDL3/SDL_keycode.h>

#include "Log.hpp"

#include "MainLayer.hpp"
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
		PushLayer(std::make_shared<MainLayer>());
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
		s_Instance = nullptr;
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
			s_Instance->m_Window->Resize(e.GetWidth(), e.GetHeight());
			return false;
			});

		dispacher.Dispatch<WindowMinimize>([](const WindowMinimize&) {
			s_Instance->m_Window->Minimize();
			return false;
			});

		dispacher.Dispatch<WindowRestore>([](const WindowRestore&) {
			s_Instance->m_Window->Restore();
			return false;
			});

		dispacher.Dispatch<KeyPressedEvent>([](const KeyPressedEvent& e) {
			if (!s_Instance->m_KeyStates[e.GetKeycode()])
			{
				s_Instance->m_KeyStates[e.GetKeycode()] = true;
			}
			else return true; // key repeat, event handled

			// close application on pressing ESC key
			if (e.GetKeycode() == SDLK_ESCAPE)
			{
				s_Instance->Stop();
				return true;
			}
			// broken at the moment
//			if (e.GetKeycode() == SDLK_F5) 
//			{
//				s_Instance->Restart();
//				return true;
//			}
			if (e.GetKeycode() == SDLK_F11)
			{
				s_Instance->m_Window->ToggleFullScreen();
				return true;
			}
			return false;
			});

		dispacher.Dispatch<KeyReleased>([](const KeyReleased& e) {
			s_Instance->m_KeyStates[e.GetKeycode()] = false;
			return false;
			});

		if (event.HasBeenHandled())
			return;

		m_LayerStack->OnEvent(event);
	}
} // namespace OWC	