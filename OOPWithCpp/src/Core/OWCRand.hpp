#pragma once
#include "core.hpp"

#include <array>
#include <immintrin.h>
#include <glm/gtc/type_ptr.hpp>


namespace OWC::Rand
{
	OWC_FORCE_INLINE Vec2 LinearFastRandVec2(const Vec2& min, const Vec2& max)
	{
		static thread_local std::array<uint64_t, 4> stats = { 123456789u, 362436069u, 521288629u, 88675123u };

		uint64_t temp = stats[3];
		uint64_t s = stats[0];
		stats[3] = stats[2];
		stats[2] = stats[1];
		stats[1] = s;

		temp ^= (temp << 22);
		temp ^= (temp >> 16);
		stats[0] = temp ^ s ^ (s >> 38);
		Vec2 randFloats = Vec2(
			static_cast<float>(stats[0] & 0xFFFFFFFF) * (1.0f / static_cast<float>(std::numeric_limits<uint32_t>::max())),
			static_cast<float>((stats[0] >> 32) & 0xFFFFFFFF) * (1.0f / static_cast<float>(std::numeric_limits<uint32_t>::max()))
		);
		return min + (max - min) * randFloats;
	}

	OWC_FORCE_INLINE Vec4 LinearFastRandVec4(const Vec4& min, const Vec4& max)
	{
		// TODO: fix MSVC shenanigans being super slow

		static thread_local std::array<glm::u32vec4, 4> states = {
			glm::u32vec4{ 123456789u, 362436069u, 521288629u, 88675123u },
			glm::u32vec4{ 5783321u, 6615241u, 1234567u, 89101112u },
			glm::u32vec4{ 42424242u, 42424243u, 42424244u, 42424245u },
			glm::u32vec4{ 987654321u, 123456789u, 192837465u, 564738291u }
		};

		glm::u32vec4 temp = states[3];

		glm::u32vec4 s = states[0];
		states[3] = states[2];
		states[2] = states[1];
		states[1] = s;

		glm::u32vec4 tempShifted{};
		tempShifted.data = _mm_sll_epi32(temp.data, _mm_set_epi64x(0, 11));
		temp.data = _mm_xor_si128(temp.data, tempShifted.data);
		tempShifted.data = _mm_srl_epi32(temp.data, _mm_set_epi64x(0, 8));
		temp.data = _mm_xor_si128(temp.data, tempShifted.data);

		tempShifted.data = _mm_srl_epi32(s.data, _mm_set_epi64x(0, 19));
		states[0] = temp ^ s ^ tempShifted;
		Vec4 randFloats = glm::vec4(states[0]) * (1.0f / static_cast<float>(std::numeric_limits<uint32_t>::max()));
		return min + (max - min) * randFloats;
	}
}