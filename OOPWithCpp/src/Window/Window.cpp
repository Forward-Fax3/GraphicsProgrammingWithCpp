#include "SDL3/SDL.h"

#include "Window.hpp"

#include "Log.hpp"


namespace OWC
{
	namespace OWCG = OWC::Graphics;

	Window::Window(const WindowProperties& properties)
		: m_Properties(properties)
	{
		SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

		m_Window.reset(SDL_CreateWindow( // Creates the window and stores it in the unique_ptr with a custom deleter
			ToCharPtr(properties.Title),
			m_Properties.Width, m_Properties.Height,
			SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY
		));

		if (m_Window == nullptr)
			Log<LogLevel::Critical>("unable to open window: {}", SDL_GetError());

		m_GraphicsContext = OWCG::GraphicsContext::CreateGraphicsContext(*m_Window);
	}

	Window::~Window()
	{
		m_GraphicsContext.reset();
		SDL_DestroyWindow(m_Window.release());
		SDL_Quit();
	}

	void Window::Update() const
	{
		m_GraphicsContext->SwapPresentImage();

#ifndef DIST
		m_GraphicsContext->FlushValidationMessages();
#endif

		PollEvents();
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