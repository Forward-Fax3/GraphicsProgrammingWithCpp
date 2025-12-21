#pragma once
#include "Core.hpp"
#include "Interval.hpp"


namespace OWC
{
	class Ray
	{
	public:
		Ray() = default;
		OWC_FORCE_INLINE explicit Ray(const Vec3& origin, const Vec3& direction) : m_Origin(origin), m_Direction(glm::normalize(direction)) {}

		const Vec3& GetOrigin() const { return m_Origin; }
		const Vec3& GetDirection() const { return m_Direction; }
		const Interval& GetHitDistanceInterval() const { return m_HitDistance; }

		Vec3 GetPointAtDistance(float t) const { return m_Origin + t * m_Direction; }

		OWC_FORCE_INLINE void SetMaxHitDistance(float maxDistance) { m_HitDistance.SetMax(maxDistance); }

	private:
		Vec3 m_Origin = Vec3(0.0);
		Vec3 m_Direction = Vec3(0.0);
		Interval m_HitDistance{ 0.001f, std::numeric_limits<float>::max() };
	};
}
