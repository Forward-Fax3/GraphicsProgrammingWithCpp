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

		OWC_FORCE_INLINE bool Contains(float value) const
		{
			return (value >= m_MinMax.x) && (value <= m_MinMax.y);
		}

	private:
 		Vec2 m_MinMax{};
	};
}
