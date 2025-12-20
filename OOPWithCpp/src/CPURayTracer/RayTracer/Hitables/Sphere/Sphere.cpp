#include "Sphere.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>


namespace OWC
{
	HitData Sphere::IsHit(const Ray& ray) const
	{
		HitData hitData;

		glm::vec3 oc = m_Center - ray.GetOrigin();

		constexpr float a = 1.0f; // ray direction is normalized so the length squared will alway be 1
		float h = glm::dot(oc, ray.GetDirection());
		float c = glm::length2(oc) - m_Radius * m_Radius;

		float discriminant = h * h - a * c;
		if (discriminant <= 0.0f)
		{
			hitData.hasHit = false;
			return hitData;
		}

		float sqrtDiscriminant = glm::sqrt(discriminant);

		float root = (h - sqrtDiscriminant) / a;
		if (!ray.GetHitDistanceInterval().Contains(root))
		{
			root = (h + sqrtDiscriminant) / a;
			if (!ray.GetHitDistanceInterval().Contains(root))
			{
				hitData.hasHit = false;
				return hitData;
			}
		}

		hitData.hasHit = true;
		hitData.t = root;
		hitData.point = ray.GetPointAtDistance(hitData.t);
		hitData.material = m_Material.get();

		Vec3 normal = (hitData.point - m_Center) / m_Radius;
		hitData.SetFaceNormal(ray, normal);

		return hitData;
	}
}