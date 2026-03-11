#pragma once
#include "Core.hpp"
#include "BaseMaterial.hpp"


namespace OWC
{
	class DefusedLight : public BaseMaterial
	{
	public:
		DefusedLight() = delete;
		explicit DefusedLight(const Colour& emitColor = Colour(1.0f), const float emitIntensity = 1.0f)
			: m_EmitColor(emitColor * emitIntensity) {}
		~DefusedLight() override = default;

		DefusedLight(const DefusedLight&) = delete;
		DefusedLight& operator=(const DefusedLight&) = delete;
		DefusedLight(DefusedLight&&) = delete;
		DefusedLight& operator=(DefusedLight&&) = delete;

		bool Scatter(Ray& /*ray*/, const HitData& /*hitData*/) const override
		{
			return false;
		}

		Colour Emitted(Ray& /*ray*/, const HitData& /*hitData*/) const override
		{
			return m_EmitColor;
		}

	private:
		Colour m_EmitColor;
	};
}
