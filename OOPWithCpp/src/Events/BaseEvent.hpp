#pragma once
#include <cstdint>
#include <string>


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
		inline std::wstring_view GetName() const { return m_Name; }
		inline std::wstring ToString() const { return m_Name; }

	protected:
		explicit BaseEvent(EventType type, std::wstring_view name)
			: m_Name(name), m_EventType(type) {}

	private:
		std::wstring m_Name;
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
		inline bool Dispatch(const std::function<bool(const EventType&)>& func) requires std::is_base_of_v<BaseEvent, EventType>
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
