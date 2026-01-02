#pragma once
#include "Core.hpp"
#include "Interval.hpp"


namespace OWC
{
	class alignas(32) Ray
	{
	public:
		Ray() = default;
		OWC_FORCE_INLINE explicit Ray(const Vec3& origin, const Vec3& direction) : m_Origin(origin), m_Direction(glm::normalize(direction)) {}

		OWC_FORCE_INLINE const Vec3& GetOrigin() const { return m_Origin; }
		OWC_FORCE_INLINE const Vec3& GetDirection() const { return m_Direction; }

		OWC_FORCE_INLINE void SetOrigin(const Vec3& origin) { m_Origin = origin; }
		OWC_FORCE_INLINE void SetDirection(const Vec3& direction) { m_Direction = glm::normalize(direction); }
		OWC_FORCE_INLINE void SetNormalizedDirection(const Vec3& nDirection) { m_Direction = nDirection; }

		Vec3 GetPointAtDistance(f32 t) const { return m_Origin + t * m_Direction; }

	private:
		Vec3 m_Origin = Vec3(0.0);
		Vec3 m_Direction = Vec3(0.0);
	};
}
