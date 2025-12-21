#pragma once
#include <cstdint>
#include <string>
#include <bit>
#include <glm/glm.hpp>

#define OWC_FORCE_INLINE GLM_INLINE


namespace OWC
{
	using bool32 = uint32_t;
	using Vec2 = glm::vec2;
	using Vec3 = glm::vec3;
	using Vec4 = glm::vec4;
	using Mat4 = glm::mat4;

	using Point = Vec3;
	using Colour = Vec4;

    [[nodiscard]] consteval bool IsDebugMode() noexcept
    {
    #ifdef NDEBUG
        return false;
    #else
        return true;
    #endif
    }

    [[nodiscard]] consteval bool IsDistributionMode() noexcept
	{
    #ifdef DIST
		return true;
    #else
		return false;
    #endif
	}

    [[nodiscard]] constexpr const char* ToCharPtr(const std::u8string& str)
    {
		return std::bit_cast<char const*>(str.c_str());
    }
}
