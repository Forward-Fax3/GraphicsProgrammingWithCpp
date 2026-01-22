#pragma once
#include "Core.hpp"

#include "Log.hpp"

#pragma warning(push)
#pragma warning(disable: 4324) // structure was padded due to alignment specifier


namespace OWC
{
	class alignas(32) Ray
	{
	public:
		Ray() = default;
		OWC_FORCE_INLINE explicit Ray(const Point& origin, const Vec3& direction) : m_Origin(origin), m_Direction(glm::normalize(direction)), m_InvDirection(1.0f / m_Direction) {}
 
		// TODO: use intrinsics to speed up copy constructor and assignment operator
 //		Ray(const Ray& ray)
 //			: m_Origin(ray.m_Origin), m_Direction(ray.m_Direction), m_InvDirection(ray.m_InvDirection)
 //		{
 //			Log<LogLevel::Debug>("Ray copy constructor");
 //		}
 //
 //		Ray& operator=(const Ray& ray)
 //		{
 //			Log<LogLevel::Debug>("Ray copy assignment operator");
 //
 //			if (this == &ray)
 //				return *this;
 //
 //			m_Origin = ray.m_Origin;
 //			m_Direction = ray.m_Direction;
 //			m_InvDirection = ray.m_InvDirection;
 //			return *this;
 //		}

		OWC_FORCE_INLINE const Vec3& GetOrigin() const { return m_Origin; }
		OWC_FORCE_INLINE const Vec3& GetDirection() const { return m_Direction; }
		OWC_FORCE_INLINE const Vec3& GetInvDirection() const { return m_InvDirection; }

		OWC_FORCE_INLINE void SetOrigin(const Vec3& origin) { m_Origin = origin; }
		OWC_FORCE_INLINE void SetDirection(const Vec3& direction) { m_Direction = glm::normalize(direction); m_InvDirection = 1.0f / m_Direction; }
		OWC_FORCE_INLINE void SetNormalizedDirection(const Vec3& nDirection) { m_Direction = nDirection; m_InvDirection = 1.0f / m_Direction; }

		Vec3 GetPointAtDistance(f32 t) const { return glm::fma(Vec3(t), m_Direction, m_Origin); }

	private:
		Point m_Origin = Point(0.0);
		Vec3 m_Direction = Vec3(0.0);
		Vec3 m_InvDirection = Vec3(0.0);
	};
}

#pragma warning(pop)
