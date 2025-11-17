#pragma once
#include "BaseEvent.hpp"


namespace OWC
{
	class WindowResize final : public BaseEvent
	{
	public:
		WindowResize(int width, int height)
			: BaseEvent(EventType::WindowResize), m_Width(width), m_Height(height) {}
		~WindowResize() override = default;

		static EventType GetStaticType() { return EventType::WindowResize; }

		inline int GetWidth() const { return m_Width; }
		inline int GetHeight() const { return m_Height; }

	private:
		int m_Width;
		int m_Height;
	};
} // namespace OWC