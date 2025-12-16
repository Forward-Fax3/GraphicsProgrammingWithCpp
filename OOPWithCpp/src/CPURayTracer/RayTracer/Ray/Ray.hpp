#pragma once
#include "core.hpp"
#include "Interval.hpp"


namespace OWC
{
	class Ray
	{
		public:
		Ray() = default;
		explicit Ray(const Vec3& origin, const Vec3& direction) : m_Origin(origin), m_Direction(direction) {}

		Vec3 GetOrigin() const { return m_Origin; }
		Vec3 GetDirection() const { return m_Direction; }

		Vec3 GetAtTime(float t) const { return m_Origin + t * m_Direction; }

	private:
		Vec3 m_Origin = Vec3(0.0);
		Vec3 m_Direction = Vec3(0.0);
		Interval m_HitDistance{ 0.0f, std::numeric_limits<float>::max() };
	};
}
