#include "Core.hpp"
#include "Hittables.hpp"


namespace OWC
{
	void Hitables::AddObject(const std::shared_ptr<BaseHittable>& newHittable)
	{
		if (const auto hittables = std::dynamic_pointer_cast<Hitables>(newHittable))
		{
			AddObjects(hittables);
			return;
		}
		m_Hitables.emplace_back(newHittable);
	}

	void Hitables::AddObjects(const std::vector<std::shared_ptr<BaseHittable>>& newHittables)
	{
		uSize newSize = m_Hitables.size() + newHittables.size();

		if (m_Hitables.capacity() < newSize)
			m_Hitables.reserve(newSize);

		for (const auto& hittable : newHittables)
			AddObject(hittable);
	}

	void Hitables::AddObjects(const std::shared_ptr<Hitables>& newHittables)
	{
		uSize newSize = m_Hitables.size() + newHittables->GetNumberOfObjects();

		if (m_Hitables.capacity() < newSize)
			m_Hitables.reserve(newSize);

		for (uSize i = 0; i < newHittables->GetNumberOfObjects(); i++)
			AddObject(newHittables->m_Hitables[i]);
	}

	HitData __vectorcall Hitables::IsHit(const Ray& ray, const Interval& range) const
	{
		Interval rayRange = range;
		HitData closestHitData;
		closestHitData.t = std::numeric_limits<f32>::max();
		closestHitData.hasHit = false;

		for (const auto& hittable : m_Hitables)
		{
			HitData hitData = hittable->IsHit(ray, rayRange);
			if (hitData.hasHit && hitData.t < closestHitData.t)
			{
				closestHitData = hitData;
				rayRange.SetMax(hitData.t);
			}
		}
		return closestHitData;
	}
}