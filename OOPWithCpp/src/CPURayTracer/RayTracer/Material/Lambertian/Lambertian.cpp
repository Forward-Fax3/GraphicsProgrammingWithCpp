#include "Lambertian.hpp"
#include "SolidTexture.hpp"
#include "OWCRand.hpp"


namespace OWC
{
	OWC_FORCE_INLINE static bool operator<(const Vec3& lhs, f32 rhs)
	{
		Vec3 absLHS(glm::abs(lhs));
		return absLHS.x < rhs && absLHS.y < rhs && absLHS.z < rhs;
	}

	Lambertian::Lambertian(const Colour& colour) : m_Texture(std::make_shared<SolidTexture>(colour)) {}

	Lambertian::Lambertian(const std::shared_ptr<BaseTexture>& texture) : m_Texture(texture) {}

	bool Lambertian::Scatter(Ray& ray, const HitData& hitData) const
	{
		Vec3 scatterDirection = hitData.normal + Rand::FastUnitVecctor();

		ray.SetOrigin(hitData.point);
		if (scatterDirection < 1e-8f)
			ray.SetNormalizedDirection(hitData.normal); // hitdata normal is normalized
		else 
			ray.SetDirection(scatterDirection); // hitdata normal is normalized so we can scale by 0.5 to keep it normalized

		return true;
	}

	Colour Lambertian::Albedo(HitData& data) const
	{
		return m_Texture->Value(data.point);
	}
}
