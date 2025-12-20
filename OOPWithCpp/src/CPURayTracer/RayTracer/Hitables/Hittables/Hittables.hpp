#pragma once
#include "BaseHittable.hpp"

#include <memory>
#include <vector>


namespace OWC
{
	class Hitables : public BaseHittable
	{
	public:
		Hitables() = default;

		void AddObject(const std::shared_ptr<BaseHittable>& newHittable);
		void AddObjects(const std::vector<std::shared_ptr<BaseHittable>>& newHittables);
		void AddObjects(const std::shared_ptr<Hitables>& newHittables);

		OWC_FORCE_INLINE void Reserve(size_t numberOfObjects) { m_Hitables.reserve(numberOfObjects); }
		OWC_FORCE_INLINE size_t GetNumberOfObjects() const { return m_Hitables.size(); }
		OWC_FORCE_INLINE void ClearObjects() { m_Hitables.clear(); }

		HitData IsHit(const Ray& ray) const override;

	private:
		std::vector<std::shared_ptr<BaseHittable>> m_Hitables;
	};
}
