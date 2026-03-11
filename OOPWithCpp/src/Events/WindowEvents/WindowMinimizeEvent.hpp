#pragma once
#include "BaseEvent.hpp"


namespace OWC
{
	class WindowMinimize
		: public BaseEvent
	{
	public:
		WindowMinimize() : BaseEvent(EventType::windowMinimize) {}
		~WindowMinimize() override = default;
		WindowMinimize(const WindowMinimize&) = default;
		WindowMinimize& operator=(const WindowMinimize&) = default;
		WindowMinimize(WindowMinimize&&) = default;
		WindowMinimize& operator=(WindowMinimize&&) = default;

		static EventType GetStaticType() { return EventType::windowMinimize; }
	};
}
