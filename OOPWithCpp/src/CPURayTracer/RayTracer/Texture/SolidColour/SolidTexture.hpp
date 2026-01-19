#pragma once
#include "Core.hpp"
#include "BaseTexture.hpp"


namespace OWC
{
	class SolidTexture : public BaseTexture
	{
	public:
		SolidTexture() = delete;
		OWC_FORCE_INLINE explicit SolidTexture(f32 greyScale) : m_Colour(Colour(greyScale)) {}
		OWC_FORCE_INLINE explicit SolidTexture(const Colour& colour) : m_Colour(colour) {}

		inline Colour Value(const Point&) const override { return m_Colour; }

	private:
		Colour m_Colour;
	};
}
