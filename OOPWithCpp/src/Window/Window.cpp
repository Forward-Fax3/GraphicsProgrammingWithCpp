#include "SDL3/SDL.h"

#include "Window.hpp"


namespace OWC
{
	Window::Window(const WindowProperties& properties)
		: m_Properties(properties)
	{
		SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

		m_Window.reset(SDL_CreateWindow( // Creates the window and stores it in the unique_ptr with a custom deleter
			SDL_iconv_wchar_utf8(properties.Title.c_str()),
			m_Properties.Width, m_Properties.Height,
			SDL_WINDOW_RESIZABLE
		));
	}

	void Window::Update()
	{
		pollEvents();
		// TODO: add the rest of the update logic (e.g., rendering, state updates)
	}

	void Window::pollEvents()
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			m_WindowEvent.EventCall(&event);
		}
	}

} // namespace OWC