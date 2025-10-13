#pragma once
#include "BaseEvent.hpp"


namespace OWC
{
	class WindowCloseEvent : public BaseEvent
	{
	public:
		inline WindowCloseEvent() : BaseEvent(EventType::WindowClose, L"WindowClose") {}
		~WindowCloseEvent() override = default;

		static EventType GetStaticType() { return EventType::WindowClose; }
	};
}
