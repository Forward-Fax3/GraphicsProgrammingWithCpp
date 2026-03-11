#pragma once
#include "Core.hpp"


namespace OWC
{
	class Interval
	{
	public:
		OWC_FORCE_INLINE explicit Interval() : m_MinMax(std::numeric_limits<f32>::max(), -std::numeric_limits<f32>::max()) {}
		OWC_FORCE_INLINE explicit Interval(f32 min, f32 max) : m_MinMax(min, max) {}
		~Interval() = default;

		Interval(const Interval&) = default;
		Interval& operator=(const Interval&) = default;
		Interval(Interval&&) = default;
		Interval& operator=(Interval&&) = default;

		OWC_FORCE_INLINE f32 GetMin() const { return m_MinMax.x; }
		OWC_FORCE_INLINE f32 GetMax() const { return m_MinMax.y; }

		OWC_FORCE_INLINE void SetMin(f32 min) { m_MinMax.x = min; }
		OWC_FORCE_INLINE void SetMax(f32 max) { m_MinMax.y = max; }

		OWC_FORCE_INLINE void SetMinMax(const Vec2& minMax) { m_MinMax = minMax; }
		OWC_FORCE_INLINE void SetMinMax(f32 min, f32 max) { m_MinMax.x = min; m_MinMax.y = max; }
		OWC_FORCE_INLINE void SetMinMax(__m128 minMax)
		{
			_mm_store_ss(&m_MinMax.x, minMax);
#if AVX512
			_mm_store_ss(&m_MinMax.y, _mm_permute_ps(minMax, _MM_PERM_DCAB));
#elif AVX2
			_mm_store_ss(&m_MinMax.y, _mm_permute_ps(minMax, _MM_SHUFFLE(3, 2, 0, 1)));
#else
			_mm_store_ss(&m_MinMax.y, _mm_shuffle_ps(minMax, minMax, _MM_SHUFFLE(3, 2, 0, 1)));
#endif
		}

		OWC_FORCE_INLINE bool Contains(f32 value) const
		{
			return m_MinMax.x <= value && value <= m_MinMax.y;
		}

		OWC_FORCE_INLINE void Increase(const Interval& other)
		{
			m_MinMax.x -= other.m_MinMax.x;
			m_MinMax.y += other.m_MinMax.y;
		}

		OWC_FORCE_INLINE void Increase(f32 delta)
		{
			m_MinMax.x -= delta * 0.5f;
			m_MinMax.y += delta * 0.5f;
		}

		OWC_FORCE_INLINE const Vec2& GetAsVector() const { return m_MinMax; }

		OWC_FORCE_INLINE f32 Size() const { return m_MinMax.y - m_MinMax.x; }

		static const Interval Empty;
		static const Interval Univers;

	private:
 		Vec2 m_MinMax{};
	};
}
