#pragma once
#include "Core.hpp"

#include <random>
#include <array>


namespace OWC::Rand
{
	static thread_local std::minstd_rand globalMtEngine;

	OWC_FORCE_INLINE Vec2 LinearFastRandVec2(const Vec2& min, const Vec2& max)
	{
		static thread_local std::uniform_real_distribution<f32> dist;
		Vec2 randFloats(
			dist(globalMtEngine),
			dist(globalMtEngine)
		);
		return min + (max - min) * randFloats;
	}

	OWC_FORCE_INLINE Vec4 LinearFastRandVec4(const Vec4& min, const Vec4& max)
	{
		static thread_local std::uniform_real_distribution<f32> dist;

		Vec4 randFloats(
			dist(globalMtEngine),
			dist(globalMtEngine),
			dist(globalMtEngine),
			dist(globalMtEngine)
		);
		return min + (max - min) * randFloats;
	}

	OWC_FORCE_INLINE Vec3 LinearFastRandVec3(const Vec3& min, const Vec3& max)
	{
		return LinearFastRandVec4(Vec4(min, 0.0f), Vec4(max, 0.0f)); // implicit conversion
	}

	OWC_FORCE_INLINE Vec3 FastUnitVector()
	{
		return glm::normalize(LinearFastRandVec3(Vec3(-1.0), Vec3(1.0)));
	}

	OWC_FORCE_INLINE Vec4 FastUnitVector4()
	{
		return glm::normalize(LinearFastRandVec4(Vec4(-1.0), Vec4(1.0)));
	}
}
