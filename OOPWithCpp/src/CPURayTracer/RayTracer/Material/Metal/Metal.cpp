#include "Metal.hpp"

#include "SolidTexture.hpp"

#include "OWCRand.hpp"


namespace OWC
{
	Metal::Metal(f32 roughness)
		: m_Texture(std::make_shared<SolidTexture>(1.0f)), m_Roughness(roughness) {}

	Metal::Metal(f32 roughness, const Colour& colour)
		: m_Texture(std::make_shared<SolidTexture>(colour)), m_Roughness(roughness) {}

	Metal::Metal(f32 roughness, const std::shared_ptr<BaseTexture>& texture)
		: m_Texture(texture), m_Roughness(roughness) {}

	bool Metal::Scatter(Ray& ray, const HitData& hitData) const
	{
		Vec3 newDirection = glm::reflect(ray.GetDirection(), hitData.normal);
		newDirection = glm::normalize(newDirection) + (Rand::FastUnitVector() * m_Roughness);

		ray = Ray(hitData.point, newDirection);
		return true;
	}

	Colour Metal::Albedo(HitData& data) const
	{
		return m_Texture->Value(data);
	}
}
