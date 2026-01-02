#pragma once
#define BaseHittable_HPP
#include "Ray.hpp"

#include "glm/glm.hpp"

#ifndef BASEMATERIAL_HPP
#include "BaseMaterial.hpp"
#endif // !BASEMATERIAL_HPP

#pragma warning(push)
#pragma warning(disable: 4324) // structure was padded due to alignment specifier


namespace OWC
{
	class BaseMaterial;

	struct alignas(64) HitData
	{
		Vec3 normal{};
		Vec3 point{};
		BaseMaterial* material = nullptr;
		f32 t = 0.0;
		bool frontFace = false;
		bool hasHit = false;

		OWC_FORCE_INLINE void SetFaceNormal(const Ray& ray, const Vec3& outwardNormal)
		{
			frontFace = glm::dot(ray.GetDirection(), outwardNormal) <= 0.0f;
			normal = frontFace ? outwardNormal : -outwardNormal;
		}

#if AVX512
		HitData() noexcept
		{
			static_assert(sizeof(HitData) == 64, "HitData size is not 64 bytes!");
			static_assert(alignof(HitData) == 64, "HitData alignment is not 64 bytes!");

			__m512i zero = _mm512_setzero_si512();
			_mm512_store_si512(this, zero);
		}

		~HitData()
		{
			static_assert(sizeof(HitData) == 64, "HitData size is not 64 bytes!");
			static_assert(alignof(HitData) == 64, "HitData alignment is not 64 bytes!");
			
			__m512i zero = _mm512_setzero_si512();
			_mm512_store_si512(this, zero);
		}

		HitData(const HitData& other) noexcept
		{
			static_assert(sizeof(HitData) == 64, "HitData size is not 64 bytes!");
			static_assert(alignof(HitData) == 64, "HitData alignment is not 64 bytes!");

			__m512i data1 = _mm512_load_si512(&other);
			_mm512_store_si512(this, data1);
		}

		HitData(const HitData&& old) noexcept
		{
			static_assert(sizeof(HitData) == 64, "HitData size is not 64 bytes!");
			static_assert(alignof(HitData) == 64, "HitData alignment is not 64 bytes!");

			__m512i data1 = _mm512_load_si512(&old);
			_mm512_store_si512(this, data1);
		}

		OWC_FORCE_INLINE HitData& operator=(const HitData& other) noexcept
		{
			// VA_IGNORE: self-assignment is safe for fixed-size aligned copy
			static_assert(sizeof(HitData) == 64, "HitData size is not 64 bytes!");
			static_assert(alignof(HitData) == 64, "HitData alignment is not 64 bytes!");

			__m512i data1 = _mm512_load_si512(&other);
			_mm512_store_si512(this, data1);
			return *this;
		}

		HitData& operator=(const HitData&& old) noexcept
		{
			// VA_IGNORE: self-assignment is safe for fixed-size aligned move
			static_assert(sizeof(HitData) == 64, "HitData size is not 64 bytes!");
			static_assert(alignof(HitData) == 64, "HitData alignment is not 64 bytes!");

			__m512i data1 = _mm512_load_si512(&old);
			_mm512_store_si512(this, data1);

			return *this;
		}

#elif AVX2
		HitData() noexcept
		{
			static_assert(sizeof(HitData) == 64, "HitData size is not 64 bytes!");
			static_assert(alignof(HitData) == 64, "HitData alignment is not 64 bytes!");

			__m256i zero = _mm256_setzero_si256();
			_mm256_store_si256(std::bit_cast<__m256i*>(this), zero);
			_mm256_store_si256(std::_Bit_cast<__m256i*>(std::_Bit_cast<u8*>(this) + 32), zero);
		}

		~HitData() noexcept
		{
			static_assert(sizeof(HitData) == 64, "HitData size is not 64 bytes!");
			static_assert(alignof(HitData) == 64, "HitData alignment is not 64 bytes!");

			__m256i zero = _mm256_setzero_si256();
			_mm256_store_si256(std::bit_cast<__m256i*>(this), zero);
			_mm256_store_si256(std::_Bit_cast<__m256i*>(std::_Bit_cast<u8*>(this) + 32), zero);
		}

		HitData(const HitData& other) noexcept
		{
			static_assert(sizeof(HitData) == 64, "HitData size is not 64 bytes!");
			static_assert(alignof(HitData) == 64, "HitData alignment is not 64 bytes!");

			__m256i data1 = _mm256_load_si256(std::bit_cast<__m256i*>(&other));
			_mm256_store_si256(std::bit_cast<__m256i*>(this), data1);

			__m256i data2 = _mm256_load_si256(std::_Bit_cast<const __m256i*>(std::_Bit_cast<const u8*>(&other) + 32));
			_mm256_store_si256(std::_Bit_cast<__m256i*>(std::_Bit_cast<u8*>(this) + 32), data2);
		}

		HitData(const HitData&& old) noexcept
		{
			static_assert(sizeof(HitData) == 64, "HitData size is not 64 bytes!");
			static_assert(alignof(HitData) == 64, "HitData alignment is not 64 bytes!");

			__m256i data1 = _mm256_load_si256(std::bit_cast<__m256i*>(&old));
			_mm256_store_si256(std::bit_cast<__m256i*>(this), data1);

			__m256i data2 = _mm256_load_si256(std::_Bit_cast<const __m256i*>(std::_Bit_cast<const u8*>(&old) + 32));
			_mm256_store_si256(std::_Bit_cast<__m256i*>(std::_Bit_cast<u8*>(this) + 32), data2);
		}

		HitData& operator=(const HitData& other) noexcept
		{
			// VA_IGNORE: self-assignment is safe for fixed-size aligned copy
			static_assert(sizeof(HitData) == 64, "HitData size is not 64 bytes!");
			static_assert(alignof(HitData) == 64, "HitData alignment is not 64 bytes!");

			__m256i data1 = _mm256_load_si256(std::bit_cast<__m256i*>(&other));
			_mm256_store_si256(std::bit_cast<__m256i*>(this), data1);

			__m256i data2 = _mm256_load_si256(std::_Bit_cast<const __m256i*>(std::_Bit_cast<const u8*>(&other) + 32));
			_mm256_store_si256(std::_Bit_cast<__m256i*>(std::_Bit_cast<u8*>(this) + 32), data2);

			return *this;
		}

		HitData& operator=(const HitData&& old) noexcept
		{
			// VA_IGNORE: self-assignment is safe for fixed-size aligned move
			static_assert(sizeof(HitData) == 64, "HitData size is not 64 bytes!");
			static_assert(alignof(HitData) == 64, "HitData alignment is not 64 bytes!");

			__m256i data1 = _mm256_load_si256(std::bit_cast<__m256i*>(&old));
			_mm256_store_si256(std::bit_cast<__m256i*>(this), data1);

			__m256i data2 = _mm256_load_si256(std::_Bit_cast<const __m256i*>(std::_Bit_cast<const u8*>(&old) + 32));
			_mm256_store_si256(std::_Bit_cast<__m256i*>(std::_Bit_cast<u8*>(this) + 32), data2);

			return *this;
		}
#else // SSE4.2
		HitData() noexcept
		{
			static_assert(sizeof(HitData) == 64, "HitData size is not 64 bytes!");
			static_assert(alignof(HitData) == 64, "HitData alignment is not 64 bytes!");

			__m128i zero = _mm_setzero_si128();
			_mm_store_si128(std::bit_cast<__m128i*>(this), zero);
			_mm_store_si128(std::_Bit_cast<__m128i*>(std::_Bit_cast<u8*>(this) + 16), zero);
			_mm_store_si128(std::_Bit_cast<__m128i*>(std::_Bit_cast<u8*>(this) + 32), zero);
			// last 16 bytes are empty/padding, no need to clear
		}

		~HitData() noexcept
		{
			static_assert(sizeof(HitData) == 64, "HitData size is not 64 bytes!");
			static_assert(alignof(HitData) == 64, "HitData alignment is not 64 bytes!");

			__m128i zero = _mm_setzero_si128();
			_mm_store_si128(std::bit_cast<__m128i*>(this), zero);
			_mm_store_si128(std::_Bit_cast<__m128i*>(std::_Bit_cast<u8*>(this) + 16), zero);
			_mm_store_si128(std::_Bit_cast<__m128i*>(std::_Bit_cast<u8*>(this) + 32), zero);
			// last 16 bytes are empty/padding, no need to clear
		}

		HitData(const HitData& other) noexcept
		{
			static_assert(sizeof(HitData) == 64, "HitData size is not 64 bytes!");
			static_assert(alignof(HitData) == 64, "HitData alignment is not 64 bytes!");

			__m128i data1 = _mm_load_si128(std::bit_cast<const __m128i*>(&other));
			_mm_store_si128(std::bit_cast<__m128i*>(this), data1);
			__m128i data2 = _mm_load_si128(std::_Bit_cast<const __m128i*>(std::_Bit_cast<const u8*>(&other) + 16));
			_mm_store_si128(std::_Bit_cast<__m128i*>(std::_Bit_cast<u8*>(this) + 16), data2);
			__m128i data3 = _mm_load_si128(std::_Bit_cast<const __m128i*>(std::_Bit_cast<const u8*>(&other) + 32));
			_mm_store_si128(std::_Bit_cast<__m128i*>(std::_Bit_cast<u8*>(this) + 32), data3);
			// last 16 bytes are empty/padding, no need to clear
		}

		HitData(const HitData&& old) noexcept
		{
			static_assert(sizeof(HitData) == 64, "HitData size is not 64 bytes!");
			static_assert(alignof(HitData) == 64, "HitData alignment is not 64 bytes!");

			__m128i data1 = _mm_load_si128(std::bit_cast<const __m128i*>(&old));
			_mm_store_si128(std::bit_cast<__m128i*>(this), data1);
			__m128i data2 = _mm_load_si128(std::_Bit_cast<const __m128i*>(std::_Bit_cast<const u8*>(&old) + 16));
			_mm_store_si128(std::_Bit_cast<__m128i*>(std::_Bit_cast<u8*>(this) + 16), data2);
			__m128i data3 = _mm_load_si128(std::_Bit_cast<const __m128i*>(std::_Bit_cast<const u8*>(&old) + 32));
			_mm_store_si128(std::_Bit_cast<__m128i*>(std::_Bit_cast<u8*>(this) + 32), data3);
			// last 16 bytes are empty/padding, no need to clear
		}

		HitData& operator=(const HitData& other) noexcept
		{
			// VA_IGNORE: self-assignment is safe for fixed-size aligned copy
			static_assert(sizeof(HitData) == 64, "HitData size is not 64 bytes!");
			static_assert(alignof(HitData) == 64, "HitData alignment is not 64 bytes!");

			__m128i data1 = _mm_load_si128(std::bit_cast<const __m128i*>(&other));
			_mm_store_si128(std::bit_cast<__m128i*>(this), data1);
			__m128i data2 = _mm_load_si128(std::_Bit_cast<const __m128i*>(std::_Bit_cast<const u8*>(&other) + 16));
			_mm_store_si128(std::_Bit_cast<__m128i*>(std::_Bit_cast<u8*>(this) + 16), data2);
			__m128i data3 = _mm_load_si128(std::_Bit_cast<const __m128i*>(std::_Bit_cast<const u8*>(&other) + 32));
			_mm_store_si128(std::_Bit_cast<__m128i*>(std::_Bit_cast<u8*>(this) + 32), data3);
			// last 16 bytes are empty/padding, no need to clear
			return *this;
		}

		HitData& operator=(const HitData&& old) noexcept
		{
			// VA_IGNORE: self-assignment is safe for fixed-size aligned move
			static_assert(sizeof(HitData) == 64, "HitData size is not 64 bytes!");
			static_assert(alignof(HitData) == 64, "HitData alignment is not 64 bytes!");

			__m128i data1 = _mm_load_si128(std::bit_cast<const __m128i*>(&old));
			_mm_store_si128(std::bit_cast<__m128i*>(this), data1);
			__m128i data2 = _mm_load_si128(std::_Bit_cast<const __m128i*>(std::_Bit_cast<const u8*>(&old) + 16));
			_mm_store_si128(std::_Bit_cast<__m128i*>(std::_Bit_cast<u8*>(this) + 16), data2);
			__m128i data3 = _mm_load_si128(std::_Bit_cast<const __m128i*>(std::_Bit_cast<const u8*>(&old) + 32));
			_mm_store_si128(std::_Bit_cast<__m128i*>(std::_Bit_cast<u8*>(this) + 32), data3);
			// last 16 bytes are empty/padding, no need to clear
			return *this;
		}
#endif // AVX512, AVX2, SSE4.2
	};

	class BaseHittable
	{
	public:
		BaseHittable() = default;
		virtual ~BaseHittable() = default;

		BaseHittable(const BaseHittable&) = delete;
		BaseHittable& operator=(const BaseHittable&) = delete;
		BaseHittable(BaseHittable&&) = delete;
		BaseHittable& operator=(BaseHittable&&) = delete;

		virtual HitData __vectorcall IsHit(const Ray& ray, const Interval& range) const = 0;
	};
}

#pragma warning(pop)
