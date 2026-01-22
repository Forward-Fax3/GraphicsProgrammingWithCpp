#pragma once
#include "Core.hpp"

#include "BaseHittable.hpp"
#include "Hittables.hpp"

#include "AABB.hpp"

#pragma warning(push)
#pragma warning(disable: 4324) // structure was padded due to alignment specifier


namespace OWC
{
    class SplitBVH : public BaseHitable
	{
	private:
		struct PRIVATE {};

    public:
        SplitBVH() = delete;
		explicit SplitBVH(const std::shared_ptr<Hitables>& hitables);
		explicit SplitBVH(std::vector<std::shared_ptr<BaseHitable>>& hitables, iSize start, iSize end, SplitBVH::PRIVATE);
        ~SplitBVH() override = default;

        SplitBVH(SplitBVH&) = delete;
        SplitBVH& operator=(SplitBVH&) = delete;
        SplitBVH(SplitBVH&&) = delete;
        SplitBVH& operator=(SplitBVH&&) = delete;

		bool __vectorcall IsHit(const Ray& ray, Interval& range, HitData& hitData) const override;

		AABB GetAABB() const override { return m_AABB; }

		Colour BackgroundColour(const Ray& ray) const override
		{
			return m_BackgroundFunction ?
				m_BackgroundFunction(ray) :
				BaseHitable::BackgroundColour(ray);
		}

		OWC_FORCE_INLINE void SetBackgroundFunction(const std::function<Colour(const Ray& ray)>& backgroundFunction)
		{
			m_BackgroundFunction = backgroundFunction;
		}

	private:
		static bool BoxComparison(const std::shared_ptr<BaseHitable>& a, const std::shared_ptr<BaseHitable>& b, AABB::Axis axis);

    private:
        AABB m_AABB;
        std::shared_ptr<BaseHitable> m_Left;
        std::shared_ptr<BaseHitable> m_Right;
		std::function<Colour(const Ray& ray)> m_BackgroundFunction;
    };
}

#pragma warning(pop)
