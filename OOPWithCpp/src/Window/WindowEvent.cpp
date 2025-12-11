#include "WindowEvent.hpp"
#include "WindowCloseEvent.hpp"
#include "WindowResize.hpp"
#include "WindowMinimizeEvent.hpp"
#include "WindowRestoreEvent.hpp"

#include <backends/imgui_impl_sdl3.h>


namespace OWC
{
	void WindowEvent::EventCall(const SDL_Event& event) const 
	{
		if (ImGui_ImplSDL3_ProcessEvent(&event) && m_WindowID != event.window.windowID)
			return;

		switch (event.type)
		{
		case SDL_EVENT_QUIT:
		case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
		{
			WindowCloseEvent closeEvent;
			m_Callback(closeEvent);
			break;
		}
		case SDL_EVENT_WINDOW_RESIZED:
		{
			WindowResize resizeEvent(event.window.data1, event.window.data2);
			m_Callback(resizeEvent);
			break;
		}
		case SDL_EVENT_WINDOW_MINIMIZED:
		{
			WindowMinimize minimizeEvent;
			m_Callback(minimizeEvent);
			break;
		}
		case SDL_EVENT_WINDOW_RESTORED:
		{
			WindowRestore restoreEvent;
			m_Callback(restoreEvent);
		}
			break;
		case SDL_EVENT_USER:
			//m_Callback();
			break;
		default:
			break;
		}
	}
} // namespace OWC
