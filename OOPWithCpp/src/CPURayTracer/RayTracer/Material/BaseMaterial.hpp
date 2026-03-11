#pragma once
#define BASEMATERIAL_HPP
#include "Core.hpp"
#include "Ray.hpp"

#ifndef BaseHittable_HPP
#include "BaseHittable.hpp"
#endif // !BaseHittable_HPP


namespace OWC
{
	struct HitData;

	class BaseMaterial
	{
	public:
		BaseMaterial() = default;
		virtual ~BaseMaterial() = default;

		BaseMaterial(const BaseMaterial&) = delete;
		BaseMaterial& operator=(const BaseMaterial&) = delete;
		BaseMaterial(BaseMaterial&&) = delete;
		BaseMaterial& operator=(BaseMaterial&&) = delete;

		virtual bool Scatter(Ray& ray, const HitData& hitData) const = 0;

		virtual Colour Emitted(Ray& /*ray*/, const HitData& /*hitData*/) const { return Colour(0.0f); }

		virtual Colour Albedo(HitData& /*data*/) const { return Colour(0.0f); }
	};
}
