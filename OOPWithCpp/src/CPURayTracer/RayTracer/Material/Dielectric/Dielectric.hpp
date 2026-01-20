#pragma once
#include "BaseMaterial.hpp"
#include "BaseTexture.hpp"

#include <memory>

namespace OWC
{
	class Dielectric : public BaseMaterial
	{
	public:
		Dielectric() = delete;
		Dielectric(f32 refractiveIndex);
		Dielectric(f32 refractiveIndex, const Colour& colour);
		Dielectric(f32 refractiveIndex, std::shared_ptr<BaseTexture> texture);
		~Dielectric() override = default;

		Dielectric(const Dielectric&) = delete;
		Dielectric& operator=(const Dielectric&) = delete;
		Dielectric(Dielectric&&) = delete;
		Dielectric& operator=(Dielectric&&) = delete;

		bool Scatter(Ray& ray, const HitData& hitData) const override;
		Colour Albedo(HitData& data) const override { return m_Texture->Value(data); }

	private:
		Point Refract(const Vec3& rayDirection, const Vec3& normal, f32 cosTheta, f32 ri) const;

		static f32 Reflectance(f32 cos, f32 refractiveIndex);

	private:
		std::shared_ptr<BaseTexture> m_Texture;
		f32 m_RefractiveIndex;
		f32 m_InverseRefractiveIndex; // Precomputed for efficiency, also does not take extra space due to alignment
	};
}
