#pragma once
#include <string>
#include <functional>
#include <SDL3/SDL.h>

#include "BaseEvent.hpp"


namespace OWC
{
	class WindowEvent
	{
	public:
		WindowEvent() = default;
		~WindowEvent() = default;
		WindowEvent(const WindowEvent&) = delete;
		WindowEvent& operator=(const WindowEvent&) = delete;
		WindowEvent(WindowEvent&&) = delete;
		WindowEvent& operator=(WindowEvent&&) = delete;

		void EventCall(const SDL_Event& event) const;

		inline void SetCallback(const std::function<void(BaseEvent&)>& callback) { m_Callback = callback; }

	private:
		std::function<void(BaseEvent&)> m_Callback = nullptr;
	};
} // namespace OWC
