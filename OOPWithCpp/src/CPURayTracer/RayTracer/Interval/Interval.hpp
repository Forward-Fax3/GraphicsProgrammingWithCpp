#pragma once
#include "core.hpp"


namespace OWC
{
	class Interval
	{
	public:
		Interval() = delete;
		inline Interval(float min, float max) : m_MinMax(min, max) {}
		~Interval() = default;

		Interval(const Interval&) = default;
		Interval& operator=(const Interval&) = default;
		Interval(Interval&&) = default;
		Interval& operator=(Interval&&) = default;

		OWC_FORCE_INLINE float GetMin() const { return m_MinMax.x; }
		OWC_FORCE_INLINE float GetMax() const { return m_MinMax.y; }

		OWC_FORCE_INLINE void SetMin(float min) { m_MinMax.x = min; }
		OWC_FORCE_INLINE void SetMax(float max) { m_MinMax.y = max; }

		OWC_FORCE_INLINE bool Contains(float value) const
		{
			return m_MinMax.x <= value && value <= m_MinMax.y;
		}

	private:
 		Vec2 m_MinMax{};
	};
}
