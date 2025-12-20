#pragma once
#define BaseHittable_HPP
#include "Ray.hpp"

#include "glm/glm.hpp"

#ifndef BASEMATERIAL_HPP
#include "BaseMaterial.hpp"
#endif // !BASEMATERIAL_HPP


namespace OWC
{
	class BaseMaterial;

	struct HitData
	{
		Vec3 normal{};
		Vec3 point{};
		BaseMaterial* material = nullptr;
		float t = 0.0;
		bool frontFace = false;
		bool hasHit = false;

		OWC_FORCE_INLINE void SetFaceNormal(const Ray& ray, const Vec3& outwardNormal)
		{
			frontFace = glm::dot(ray.GetDirection(), outwardNormal) <= 0.0f;
			normal = frontFace ? outwardNormal : -outwardNormal;
		}
	};

	class BaseHittable
	{
	public:
		BaseHittable() = default;
		virtual ~BaseHittable() = default;

		BaseHittable(const BaseHittable&) = delete;
		BaseHittable& operator=(const BaseHittable&) = delete;
		BaseHittable(BaseHittable&&) = delete;
		BaseHittable& operator=(BaseHittable&&) = delete;

		virtual HitData IsHit(const Ray& ray) const = 0;
	};
}
