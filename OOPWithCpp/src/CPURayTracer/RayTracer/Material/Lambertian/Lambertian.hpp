#pragma once
#include "Core.hpp"
#include "BaseMaterial.hpp"
#include "BaseTexture.hpp"

#include <memory>


namespace OWC
{
	class Lambertian : public BaseMaterial
	{
	public:
		Lambertian() = delete;
		explicit Lambertian(const Colour& colour);
		explicit Lambertian(const std::shared_ptr<BaseTexture>& texture);

		bool Scatter(Ray& ray, const HitData& hitData) const override;

		Colour Albedo(HitData& data) const override;

	private:
		std::shared_ptr<BaseTexture> m_Texture;
	};
}
