#pragma once
#include "Core.hpp"

#include "Log.hpp"

#pragma warning(push)
#pragma warning(disable: 4324) // structure was padded due to alignment specifier


namespace OWC
{
	class alignas(64) Ray
	{
	public:
		OWC_FORCE_INLINE explicit Ray(const Point& origin, const Vec3& direction) : m_Origin(origin), m_Direction(glm::normalize(direction)), m_InvDirection(1.0f / m_Direction) {}

#if AVX512 & 1
		Ray() noexcept
		{
			static_assert(sizeof(Ray) == 64, "Ray size is not 64 bytes!");
			static_assert(alignof(Ray) == 64, "Ray alignment is not 64 bytes!");

			__m512i zero = _mm512_setzero_si512();
			_mm512_store_si512(this, zero);
		}

		~Ray()
		{
			static_assert(sizeof(Ray) == 64, "Ray size is not 64 bytes!");
			static_assert(alignof(Ray) == 64, "Ray alignment is not 64 bytes!");

			__m512i zero = _mm512_setzero_si512();
			_mm512_store_si512(this, zero);
		}

		Ray(const Ray& other) noexcept
		{
			static_assert(sizeof(Ray) == 64, "Ray size is not 64 bytes!");
			static_assert(alignof(Ray) == 64, "Ray alignment is not 64 bytes!");

			__m512i data1 = _mm512_load_si512(&other);
			_mm512_store_si512(this, data1);
		}

		Ray(const Ray&& old) noexcept
		{
			static_assert(sizeof(Ray) == 64, "Ray size is not 64 bytes!");
			static_assert(alignof(Ray) == 64, "Ray alignment is not 64 bytes!");

			__m512i data1 = _mm512_load_si512(&old);
			_mm512_store_si512(this, data1);
		}

		OWC_FORCE_INLINE Ray& operator=(const Ray& other) noexcept
		{
			// VA_IGNORE: self-assignment is safe for fixed-size aligned copy
			static_assert(sizeof(Ray) == 64, "Ray size is not 64 bytes!");
			static_assert(alignof(Ray) == 64, "Ray alignment is not 64 bytes!");

			__m512i data1 = _mm512_load_si512(&other);
			_mm512_store_si512(this, data1);
			return *this;
		}

		Ray& operator=(const Ray&& old) noexcept
		{
			// VA_IGNORE: self-assignment is safe for fixed-size aligned move
			static_assert(sizeof(Ray) == 64, "Ray size is not 64 bytes!");
			static_assert(alignof(Ray) == 64, "Ray alignment is not 64 bytes!");

			__m512i data1 = _mm512_load_si512(&old);
			_mm512_store_si512(this, data1);

			return *this;
		}
#else
		Ray() = default;
#endif

		OWC_FORCE_INLINE const Vec3& GetOrigin() const { return m_Origin; }
		OWC_FORCE_INLINE const Vec3& GetDirection() const { return m_Direction; }
		OWC_FORCE_INLINE const Vec3& GetInvDirection() const { return m_InvDirection; }

		OWC_FORCE_INLINE void SetOrigin(const Vec3& origin) { m_Origin = origin; }
		OWC_FORCE_INLINE void SetDirection(const Vec3& direction) { m_Direction = glm::normalize(direction); m_InvDirection = 1.0f / m_Direction; }
		OWC_FORCE_INLINE void SetNormalizedDirection(const Vec3& nDirection) { m_Direction = nDirection; m_InvDirection = 1.0f / m_Direction; }

		Vec3 GetPointAtDistance(f32 t) const { return glm::fma(Vec3(t), m_Direction, m_Origin); }

	private:
		Point m_Origin = Point(0.0);
		Vec3 m_Direction = Vec3(0.0);
		Vec3 m_InvDirection = Vec3(0.0);
	};
}

#pragma warning(pop)
