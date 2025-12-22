#pragma once
#include "Core.hpp"
#include "BaseEvent.hpp"


namespace OWC
{
	class WindowResize final : public BaseEvent
	{
	public:
		WindowResize(u32 width, u32 height)
			: BaseEvent(EventType::WindowResize), m_NewSize(width, height) {}

		OWC_FORCE_INLINE static EventType GetStaticType() { return EventType::WindowResize; }

		OWC_FORCE_INLINE u32 GetWidth() const { return m_NewSize.x; }
		OWC_FORCE_INLINE u32 GetHeight() const { return m_NewSize.y; }
		OWC_FORCE_INLINE Vec2u GetSize() const { return m_NewSize; }

	private:
		Vec2u m_NewSize;
	};
} // namespace OWC