#pragma once
#include "BaseHittable.hpp"
#include "Ray.hpp"

#include <glm/vec3.hpp>


namespace OWC
{
	class Sphere : public BaseHittable
	{
	public:
		Sphere() = delete;
		inline Sphere(const glm::vec3& center, float radius) : m_Center(center), m_Radius(radius) {}
		~Sphere() override = default;

		Sphere(const Sphere&) = delete;
		Sphere& operator=(const Sphere&) = delete;
		Sphere(Sphere&&) = delete;
		Sphere& operator=(Sphere&&) = delete;

		virtual HitData IsHit(const Ray& ray) const override;

	private:
		glm::vec3 m_Center;
		float m_Radius;
	};
}
