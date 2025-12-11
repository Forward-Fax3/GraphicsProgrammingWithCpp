#pragma once
#include <functional>
#include <memory>

#include "BaseEvent.hpp"
#include "WindowEvent.hpp"
#include "GraphicsContext.hpp"


namespace OWC
{
	struct WindowProperties
	{
		std::u8string Title;
		int Width;
		int Height;
	};

	class Window
	{
	public:
		Window() = delete;
		explicit Window(const WindowProperties& properties);
		~Window();

		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;
		Window(Window&&) = delete;
		Window& operator=(Window&&) = delete;

		void Update() const;

		inline void SetEventCallback(const std::function<void(BaseEvent&)>& callback) { m_WindowEvent->SetCallback(callback); }

		Graphics::GraphicsContext& GetGraphicsContext() const { return *m_GraphicsContext; }
		std::weak_ptr<Graphics::GraphicsContext> GetGraphicsContextPtr() const { return m_GraphicsContext; }

		int GetWidth() const { return m_Properties.Width; }
		int GetHeight() const { return m_Properties.Height; }

		bool IsWindowMinimized() const { return m_IsMinimized; }

		void Resize(int width, int height);
		void Minimize();
		void Restore();

		void ImGuiInit() const;
		void ImGuiShutdown() const;

		void ImGuiNewFrame() const;

	private:
		void PollEvents() const;

	private:
		std::unique_ptr<WindowEvent> m_WindowEvent;
		WindowProperties m_Properties;
		std::unique_ptr<SDL_Window, decltype([](SDL_Window* windowPtr){ SDL_DestroyWindow(windowPtr); })> m_Window = nullptr;
		std::shared_ptr<Graphics::GraphicsContext> m_GraphicsContext = nullptr;
		bool m_IsMinimized = false;
	};
}
