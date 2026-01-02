#include "Sphere.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>


namespace OWC
{
	HitData __vectorcall Sphere::IsHit(const Ray& ray, const Interval& range) const
	{
		HitData hitData;

		Vec3 oc = m_Center - ray.GetOrigin();

		constexpr f32 a = 1.0f; // ray direction is normalized so the length squared will alway be 1
		f32 h = glm::dot(oc, ray.GetDirection());
		f32 c = glm::length2(oc) - m_Radius * m_Radius;

		f32 discriminant = h * h - a * c;
		if (discriminant <= 0.0f)
		{
			hitData.hasHit = false;
			return hitData;
		}

		f32 sqrtDiscriminant = glm::sqrt(discriminant);

		f32 root = (h - sqrtDiscriminant) / a;
		if (!range.Contains(root))
		{
			root = (h + sqrtDiscriminant) / a;
			if (!range.Contains(root))
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