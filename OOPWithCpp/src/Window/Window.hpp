#pragma once
#include <functional>
#include <memory>

#include "BaseEvent.hpp"
#include "WindowEvent.hpp"


namespace OWC
{
	struct WindowProperties
	{
		std::wstring Title;
		int Width;
		int Height;
	};

	class Window
	{
	public:
		Window() = delete;
		explicit Window(const WindowProperties& properties);
		~Window() = default;

		void Update();

		inline void SetEventCallback(const std::function<void(BaseEvent&)>& callback) { m_WindowEvent.SetCallback(callback); }

	private:
		void pollEvents();

	private:
		WindowEvent m_WindowEvent;
		WindowProperties m_Properties;
		std::unique_ptr<SDL_Window, decltype([](SDL_Window* windowPtr){ SDL_DestroyWindow(windowPtr); })> m_Window;
	};
}
