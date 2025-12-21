#pragma once
#include "Core.hpp"


namespace OWC
{
	class BaseTexture
	{
	public:
		BaseTexture() = default;
		virtual ~BaseTexture() = default;
		virtual Colour Value(const Point& p) const = 0;
	};
}
