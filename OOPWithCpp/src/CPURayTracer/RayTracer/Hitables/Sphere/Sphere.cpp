#include "Sphere.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>


namespace OWC
{
	HitData Sphere::IsHit(const Ray& ray) const
	{
		HitData hitData{};

		glm::vec3 oc = ray.GetOrigin() - m_Center;

		float a = glm::length2(ray.GetDirection());
		float h = glm::dot(oc, ray.GetDirection());
		float c = glm::length2(oc) - m_Radius * m_Radius;

		float discriminant = h * h - a * c;
		if (discriminant < 0.0f)
		{
			hitData.hasHit = false;
			return hitData;
		}

		// for now only test if there is a hit, not where
		hitData.hasHit = true;
		return hitData;

//		float sqrtDiscriminant = glm::sqrt(discriminant);
//		float t1 = (-b - sqrtDiscriminant) / (2.0f * a);
//		float t2 = (-b + sqrtDiscriminant) / (2.0f * a);
//		hitData.t = (t1 >= 0.0f) ? t1 : ((t2 >= 0.0f) ? t2 : -1.0f);
//		hitData.hasHit = (hitData.t >= 0.0f);
//		return hitData;
	}
}