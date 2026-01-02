#pragma once
#include "BaseHittable.hpp"
#include "Ray.hpp"

#include <glm/vec3.hpp>

#include <memory>


namespace OWC
{
	class Sphere : public BaseHittable
	{
	public:
		Sphere() = delete;
		OWC_FORCE_INLINE Sphere(const Vec3& center, f32 radius, const std::shared_ptr<BaseMaterial>& mat) : m_Center(center), m_Material(mat), m_Radius(radius) {}
		~Sphere() override = default;

		Sphere(const Sphere&) = delete;
		Sphere& operator=(const Sphere&) = delete;
		Sphere(Sphere&&) = delete;
		Sphere& operator=(Sphere&&) = delete;

		HitData __vectorcall IsHit(const Ray& ray, const Interval& range) const override;

	private:
		Vec3 m_Center;
		std::shared_ptr<BaseMaterial> m_Material;
		f32 m_Radius;
	};
}
