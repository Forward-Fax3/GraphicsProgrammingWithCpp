#pragma once
#include "Core.hpp"
#include "BaseHittable.hpp"


namespace OWC
{
	class BaseTexture
	{
	public:
		BaseTexture() = default;
		virtual ~BaseTexture() = default;
		virtual Colour Value(const HitData& p) const = 0;
	};
}
