#include "SDL3/SDL.h"

#include "Window.hpp"


namespace OWC
{
	namespace OWCG = OWC::Graphics;

	Window::Window(const WindowProperties& properties)
		: m_Properties(properties)
	{
		SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

		m_Window.reset(SDL_CreateWindow( // Creates the window and stores it in the unique_ptr with a custom deleter
			SDL_iconv_wchar_utf8(properties.Title.c_str()),
			m_Properties.Width, m_Properties.Height,
			SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY
		));

		{
			auto temp = OWCG::GraphicsContext::CreateGraphicsContext(*m_Window);
			m_GraphicsContext.swap(temp);
		}

	}

	void Window::Update() const
	{
		PollEvents();
		// TODO: add the rest of the update logic (e.g., rendering, state updates)
	}

	bool Window::Resize(int width, int height)
	{
		if (m_Window)
		{
			SDL_SetWindowSize(m_Window.get(), width, height);
			m_Properties.Width = width;
			m_Properties.Height = height;
			return true;
		}
		return false;
	}

	void Window::PollEvents() const
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			m_WindowEvent.EventCall(event);
		}
	}
} // namespace OWC