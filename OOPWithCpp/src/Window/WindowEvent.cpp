#include "WindowEvent.hpp"
#include "WindowCloseEvent.hpp"


namespace OWC
{
	void WindowEvent::EventCall(SDL_Event* event)
	{
		// ImGui_ImplSDL3_ProcessEvent(event);

		switch (event->type)
		{
		case SDL_EVENT_QUIT:
		{
			WindowCloseEvent closeEvent;
			m_Callback(closeEvent);
			break;
		}
		case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
			//m_Callback();
			break;
		case SDL_EVENT_WINDOW_RESIZED:
			//m_Callback();
			break;
		case SDL_EVENT_WINDOW_MOVED:
			//m_Callback();
			break;
		case SDL_EVENT_WINDOW_FOCUS_GAINED:
			//m_Callback();
			break;
		case SDL_EVENT_WINDOW_FOCUS_LOST:
			//m_Callback();
			break;
		default:
			break;
		}
	}
} // namespace OWC
