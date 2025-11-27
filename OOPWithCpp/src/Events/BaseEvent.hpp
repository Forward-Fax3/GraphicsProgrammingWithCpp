#pragma once
#include <cstdint>
#include <functional>


namespace OWC
{
	enum class EventType : uint8_t
	{
		// TODO: possibly add restart event
		None = 0,
		WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
		KeyPressed, KeyReleased, KeyTyped,
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
	};

	class EventDispatcher;

	class BaseEvent
	{
	public:
		BaseEvent() = delete;
		virtual ~BaseEvent() = default;

		inline EventType GetEventType() const { return m_EventType; }
		inline bool HasBeenHandled() const { return m_HasBeenHandled; }

	protected:
		explicit BaseEvent(EventType type)
			: m_EventType(type) {}

	private:
		EventType m_EventType;
		bool m_HasBeenHandled = false;

		friend class EventDispatcher;
	};

	class EventDispatcher
	{
	public:
		EventDispatcher() = delete;
		explicit EventDispatcher(BaseEvent& event)
			: m_Event(event) {
		}

		template <typename EventType>
		requires (std::is_base_of_v<BaseEvent, EventType> && !std::is_same_v<BaseEvent, EventType>) // EventType must be a derived class of BaseEvent and not BaseEvent itself
		inline bool Dispatch(const std::function<bool(const EventType&)>& func)
		{
			if (!m_Event.m_HasBeenHandled && m_Event.GetEventType() == EventType::GetStaticType())
			{
				m_Event.m_HasBeenHandled = func(dynamic_cast<EventType&>(m_Event));
				return true;
			}
			return false;
		}

	private:
		BaseEvent& m_Event;
	};
}
