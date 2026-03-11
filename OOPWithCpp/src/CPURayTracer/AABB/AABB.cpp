#include "AABB.hpp"

#include <glm/glm.hpp>


namespace OWC
{
	AABB::AABB(const Interval& interval)
		: m_XInterval(interval), m_YInterval(interval), m_ZInterval(interval) { FinishIntervalSetup(); }

	AABB::AABB(const Interval& x, const Interval& y, const Interval& z)
		: m_XInterval(x), m_YInterval(y), m_ZInterval(z) { FinishIntervalSetup(); }

	AABB::AABB(const Point& a, const Point& b)
	{
		m_XInterval = (a.x <= b.x) ? Interval(a.x, b.x) : Interval(b.x, a.x);
		m_YInterval = (a.y <= b.y) ? Interval(a.y, b.y) : Interval(b.y, a.y);
		m_ZInterval = (a.z <= b.z) ? Interval(a.z, b.z) : Interval(b.z, a.z);
		FinishIntervalSetup();
	}

	AABB::AABB(const AABB& box0, const AABB& box1)
	{
		m_XInterval = Interval(glm::min(box0.m_XInterval.GetMin(), box1.m_XInterval.GetMin()),
							   glm::max(box0.m_XInterval.GetMax(), box1.m_XInterval.GetMax()));

		m_YInterval = Interval(glm::min(box0.m_YInterval.GetMin(), box1.m_YInterval.GetMin()),
							   glm::max(box0.m_YInterval.GetMax(), box1.m_YInterval.GetMax()));

		m_ZInterval = Interval(glm::min(box0.m_ZInterval.GetMin(), box1.m_ZInterval.GetMin()),
							   glm::max(box0.m_ZInterval.GetMax(), box1.m_ZInterval.GetMax()));

		FinishIntervalSetup();
	}

	bool AABB::IsBigger(const AABB& otherAABB) const
	{
		bool returnBool = true;
		returnBool &= this->m_XInterval.GetMin() <= otherAABB.m_XInterval.GetMin() && this->m_XInterval.GetMax() >= otherAABB.m_XInterval.GetMax();
		returnBool &= this->m_YInterval.GetMin() <= otherAABB.m_YInterval.GetMin() && this->m_YInterval.GetMax() >= otherAABB.m_YInterval.GetMax();
		returnBool &= this->m_ZInterval.GetMin() <= otherAABB.m_ZInterval.GetMin() && this->m_ZInterval.GetMax() >= otherAABB.m_ZInterval.GetMax();
		return returnBool;
	}

	void AABB::Expand(const AABB& newAABB)
	{
		m_XInterval = Interval(glm::min(m_XInterval.GetMin(), newAABB.m_XInterval.GetMin()),
							   glm::max(m_XInterval.GetMax(), newAABB.m_XInterval.GetMax()));

		m_YInterval = Interval(glm::min(m_YInterval.GetMin(), newAABB.m_YInterval.GetMin()),
							   glm::max(m_YInterval.GetMax(), newAABB.m_YInterval.GetMax()));

		m_ZInterval = Interval(glm::min(m_ZInterval.GetMin(), newAABB.m_ZInterval.GetMin()),
							   glm::max(m_ZInterval.GetMax(), newAABB.m_ZInterval.GetMax()));

		FinishIntervalSetup();
	}

	void AABB::MinimumPadding()
	{
		const f32 minPadding = 0.0001f;
		if (m_XInterval.Size() < minPadding)
			m_XInterval.Increase(minPadding);
		if (m_YInterval.Size() < minPadding)
			m_YInterval.Increase(minPadding);
		if (m_ZInterval.Size() < minPadding)
			m_ZInterval.Increase(minPadding);
	}

	void AABB::SetLongestAxis()
	{
		f32 xSize = m_XInterval.Size();
		f32 ySize = m_YInterval.Size();
		f32 zSize = m_ZInterval.Size();

		if (xSize >= ySize && xSize >= zSize)
			m_LongestAxis = Axis::x;
		else if (ySize >= xSize && ySize >= zSize)
			m_LongestAxis = Axis::y;
		else
			m_LongestAxis = Axis::z;
	}

	const AABB AABB::Empty(Interval::Empty);

	const AABB AABB::Univers(Interval::Univers);
}
