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

		void Update() const;

		inline void SetEventCallback(const std::function<void(BaseEvent&)>& callback) { m_WindowEvent.SetCallback(callback); }

		bool Resize(int width, int height);

		Graphics::GraphicsContext& GetGraphicsContext() const { return *m_GraphicsContext; }
		std::weak_ptr<Graphics::GraphicsContext> GetGraphicsContextPtr() const { return m_GraphicsContext; }

		int GetWidth() const { return m_Properties.Width; }
		int GetHeight() const { return m_Properties.Height; }

		bool IsWindowMinimized() const { return m_IsMinimized; }

		void Minimize();
		void Restore();

		void ImGuiInit() const;
		void ImGuiShutdown() const;

		void ImGuiNewFrame() const;

	private:
		void PollEvents() const;

	private:
		WindowEvent m_WindowEvent;
		WindowProperties m_Properties;
		std::unique_ptr<SDL_Window, decltype([](SDL_Window* windowPtr){ SDL_DestroyWindow(windowPtr); })> m_Window = nullptr;
		std::shared_ptr<Graphics::GraphicsContext> m_GraphicsContext = nullptr;
		bool m_IsMinimized = false;
	};
}
