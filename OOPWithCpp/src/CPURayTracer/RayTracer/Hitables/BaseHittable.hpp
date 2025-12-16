#pragma once
#include "Ray.hpp"


namespace OWC
{
	struct HitData
	{
		float t;
		bool frontFace;
		bool hasHit;
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
