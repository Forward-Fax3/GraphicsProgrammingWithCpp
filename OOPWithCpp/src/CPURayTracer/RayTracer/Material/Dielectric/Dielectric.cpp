#include "Dielectric.hpp"
#include "SolidTexture.hpp"

#include "OWCRand.hpp"

#include <glm/gtx/norm.hpp>


namespace OWC
{
	Dielectric::Dielectric(f32 refractiveIndex)
		: m_Texture(std::make_shared<SolidTexture>(1.0f)), m_RefractiveIndex(refractiveIndex),
		  m_InverseRefractiveIndex(1.0f / refractiveIndex) {}

	Dielectric::Dielectric(f32 refractiveIndex, const Colour& colour)
		: m_Texture(std::make_shared<SolidTexture>(colour)), m_RefractiveIndex(refractiveIndex),
		m_InverseRefractiveIndex(1.0f / refractiveIndex) {
	}

	Dielectric::Dielectric(f32 refractiveIndex, std::shared_ptr<BaseTexture> texture)
		: m_Texture(texture), m_RefractiveIndex(refractiveIndex),
		m_InverseRefractiveIndex(1.0f / refractiveIndex) {
	}

	bool Dielectric::Scatter(Ray& ray, const HitData& hitData) const
	{
		f32 ri = hitData.frontFace ? m_InverseRefractiveIndex : m_RefractiveIndex;

		f32 cosTheta = glm::min(glm::dot(-ray.GetDirection(), hitData.normal), 1.0f);
		f32 sinTheta = glm::sqrt(1.0f - cosTheta * cosTheta);

		Vec3 newDirection{};
		Vec2 random = Rand::LinearFastRandVec2(Vec2(0.0f), Vec2(1.0f));
		if ((ri * sinTheta > 1.0f) || Reflectance(cosTheta, ri) > random.x)
			newDirection = glm::reflect(ray.GetDirection(), hitData.normal);
		else
			newDirection = Refract(ray.GetDirection(), hitData.normal, cosTheta, ri);

		ray = Ray(hitData.point, newDirection);
		return true;
	}

	Point Dielectric::Refract(const Vec3& rayDirection, const Vec3& normal, f32 cosTheta, f32 ri) const
	{
		Vec3 rOutPerp = ri * (rayDirection + cosTheta * normal);
		Vec3 rOutParallel = -std::sqrt(glm::abs(1.0f - glm::length2(rOutPerp))) * normal;
		return rOutPerp + rOutParallel;
	}

	f32 Dielectric::Reflectance(f32 cos, f32 refractiveIndex)
	{
		f32 r0 = (1.0f - refractiveIndex) / (1.0f + refractiveIndex);
		r0 *= r0;
		return r0 + (1.0f - r0) * glm::pow(1.0f - cos, 5.0f);
	}
}