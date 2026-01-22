#pragma once
#include "Core.hpp"

#include "Ray.hpp"
#include "Interval.hpp"

#include <array>

#define SIMD 1

#pragma warning(push)
#pragma warning(disable: 4324) // structure was padded due to alignment specifier


namespace OWC
{
	class alignas(32) AABB
	{
	public:
		enum class Axis : uint8_t { x = 0, y, z, none };

	public:
		explicit AABB() = default;
		AABB(const AABB& other);
		explicit AABB(const Interval& interval);
		explicit AABB(const Interval& x, const Interval& y, const Interval& z);
		explicit AABB(const Point& a, const Point& b);
		explicit AABB(const AABB& box0, const AABB& box1);

		~AABB() = default;

		AABB& operator=(const AABB& aabb) = default;

		bool IsBigger(const AABB& otherAABB) const;

		const Interval& GetAxisInterval(const Axis& axis) const;

		OWC_FORCE_INLINE bool __vectorcall IsHit(const Ray& ray, Interval rayT) const;

		void Expand(const AABB& newAABB);

		inline double GetSurfaceArea() const { return 2.0 * (m_XInterval.Size() * m_YInterval.Size() + m_XInterval.Size() * m_ZInterval.Size() + m_YInterval.Size() * m_ZInterval.Size()); }

		OWC_FORCE_INLINE AABB::Axis LongestAxis() const { return m_LongestAxis; }

		static const AABB Empty;
		static const AABB Univers;

	private:
		void MinimumPadding();
		void SetLongestAxis();
		OWC_FORCE_INLINE void FinishIntervalSetup() { MinimumPadding(); SetLongestAxis(); }

	private:
		Interval m_XInterval;
		Interval m_YInterval;
		Interval m_ZInterval;
		Axis m_LongestAxis = Axis::none;
	};

	OWC_FORCE_INLINE AABB::Axis& operator++(AABB::Axis& axis)
	{
#if _HAS_CXX23
		axis = static_cast<AABB::Axis>(std::to_underlying(axis) + 1);
#else
		axis = static_cast<AABB::Axis>(static_cast<uint8_t>(axis) + 1);
#endif
		return axis;
	}

	OWC_FORCE_INLINE AABB::Axis operator++(AABB::Axis& axis, int)
	{
		AABB::Axis oldAxis = axis;
		++axis;
		return oldAxis;
	}

	static constexpr std::underlying_type_t<AABB::Axis> operator+(AABB::Axis axis) noexcept
	{
#if _HAS_CXX23
		return std::to_underlying(axis);
#else
		return static_cast<std::underlying_type_t<AABB::Axis>>(axis);
#endif
	}

	OWC_FORCE_INLINE bool __vectorcall AABB::IsHit(const Ray& ray, Interval rayT) const
	{
#if AVX2 & SIMD
		const __m256i AVX2i32_FloatLoadPermutationIndex = _mm256_setr_epi32(0, 0, 1, 1, 2, 2, 3, 3);

		const __m256 AVX2f32_AltNegXOR = _mm256_setr_ps(0.0f, -0.0f, 0.0f, -0.0f, 0.0f, -0.0f, 0.0f, -0.0f);


		// load m_XInterval, m_YInterval and m_ZInterval into an AVX2 register
		__m256 AVX2f32_AxisBounds = _mm256_load_ps(std::bit_cast<f32*>(this));

		// Load ray origin into an AVX2 Register and double each axis into 128 bit lanes
		__m256 AVX2f32_RayOrigin = _mm256_permutevar8x32_ps(_mm256_castps128_ps256(ray.GetOrigin().data), AVX2i32_FloatLoadPermutationIndex);

		// Load ray inverted direction into an AVX2 Register and double each axis into 128 bit lanes
		__m256 AVX2f32_RayInvDirection = _mm256_permutevar8x32_ps(_mm256_castps128_ps256(ray.GetInvDirection().data), AVX2i32_FloatLoadPermutationIndex);


		// create T in AVX2 register
		__m256 AVX2f32_T = _mm256_mul_ps(_mm256_sub_ps(AVX2f32_AxisBounds, AVX2f32_RayOrigin), AVX2f32_RayInvDirection);

		// creates the swapped T register
#if AVX512
		__m256 AVX2f32_T128BitSwaped = _mm256_permute_ps(AVX2f32_T, _MM_PERM_CDAB);
#else // !AVX512
		__m256 AVX2f32_T128BitSwaped = _mm256_permute_ps(AVX2f32_T, _MM_SHUFFLE(2, 3, 0, 1));
#endif // AVX512


		// performs an xor so that min and max operations can be performed
		__m256 AVX2f32_AltNegT = _mm256_xor_ps(AVX2f32_T, AVX2f32_AltNegXOR);
		__m256 AVX2f32_AltNegSwapedT = _mm256_xor_ps(AVX2f32_T128BitSwaped, AVX2f32_AltNegXOR);

		// minimums AVX2f32_AltNegT and AVX2f32_AltNegSwapedT so that AVX2f32_MinNegMaxT is (Xmin, -Xmax, Ymin, -Ymax, Zmin, -Zmax, Amin, -Amax)
		__m256 AVX2f32_MinNegMaxT = _mm256_min_ps(AVX2f32_AltNegT, AVX2f32_AltNegSwapedT);


		// creates a m128_AltNegTest register then shrinks it to the smallest size
#if AVX512
		__m128 SSEf32_lane1MinMax = _mm_permute_ps(_mm256_castps256_ps128(AVX2f32_MinNegMaxT), _MM_PERM_BADC);
#else // !AVX512
		__m128 SSEf32_lane1MinMax = _mm_permute_ps(_mm256_castps256_ps128(AVX2f32_MinNegMaxT), _MM_SHUFFLE(1, 0, 3, 2));
#endif // AVX512
		__m128 SSEf32_lane2MinMax = _mm256_extractf128_ps(AVX2f32_MinNegMaxT, 1);
		__m128 SSEf32_AltNegTest = _mm_max_ps(_mm256_castps256_ps128(AVX2f32_MinNegMaxT), SSEf32_lane1MinMax);
		SSEf32_AltNegTest = _mm_max_ps(SSEf32_AltNegTest, SSEf32_lane2MinMax);

		// get rayT and perform max with that as well to make sure that MinNegMaxT is inside the rays boundary
		__m128 SEEf32_IntevalLimits = _mm_setr_ps(rayT.GetMin(), rayT.GetMax(), 0.0f, 0.0f);
		__m128 SSEf32_AltNegIntevalLimits = _mm_xor_ps(SEEf32_IntevalLimits, _mm256_castps256_ps128(AVX2f32_AltNegXOR));
		SSEf32_AltNegTest = _mm_max_ps(SSEf32_AltNegTest, SSEf32_AltNegIntevalLimits);

		// inverts the values back for final comparison
		__m128 SSEf32_Test = _mm_xor_ps(SSEf32_AltNegTest, _mm256_castps256_ps128(AVX2f32_AltNegXOR));

		// test the boundary of the m128_Test minimum and maximum to make sure max is not smaller than or equal to the minimum bound
		// using shufpd and comisd instructions as other wise the compiler will do the same thing in memory instead
		// of just using built in instruction
		f32 min = _mm_cvtss_f32(SSEf32_Test);
#if AVX512
		f32 max = _mm_cvtss_f32(_mm_permute_ps(SSEf32_Test, _MM_PERM_DCAB));
#else // !AVX512
		f32 max = _mm_cvtss_f32(_mm_permute_ps(SSEf32_Test, _MM_SHUFFLE(3, 2, 0, 1)));
#endif // AVX512
		return min < max;

#else // !(AVX2 & SIMD)
		for (Axis axis = Axis::x; axis <= Axis::z; axis++)
		{
			const Interval& axisBounds = GetAxisInterval(axis);
			const f32 rAxisOrigin = ray.GetOrigin()[+axis];
			const f32 rAxisInvDirection = ray.GetInvDirection()[+axis];

			Vec2 t = (axisBounds.GetAsVector() - rAxisOrigin) * rAxisInvDirection;

			if (t.x > t.y)
				std::swap(t.x, t.y);

#if SIMD
			const __m128 SEEf32_InvertValue = _mm_setr_ps(0.0f, -0.0f, 0.0f, -0.0f);

			// load t and rayT into SIMD register while negating y so that the same comparison can be computed on x and y
			__m128 SEEf32_T = _mm_setr_ps(t.x, t.y, 0.0f, 0.0f);
			__m128 SEEf32_AltNegT = _mm_xor_ps(SEEf32_InvertValue, SEEf32_T);
			__m128 SEEf32_IntevalLimits = _mm_setr_ps(rayT.GetMin(), rayT.GetMax(), 0.0f, 0.0f);
			__m128 SEEf32_RayT = _mm_xor_ps(SEEf32_InvertValue, SEEf32_IntevalLimits);

			// perform comparison
			__m128 SEEf32_BitMask = _mm_cmpgt_ps(SEEf32_AltNegT, SEEf32_RayT);

			// load based on mask rayT or T into register

			rayT.SetMinMax(_mm_blendv_ps(SEEf32_IntevalLimits, SEEf32_T, SEEf32_BitMask));

#else // !SIMD
			if (t.x > rayT.GetMin())
				rayT.SetMin(t.x);
			if (t.y < rayT.GetMax())
				rayT.SetMax(t.y);
#endif // SIMD

			if (rayT.GetMin() >= rayT.GetMax())
				return false;
		}
		return true;
#endif // AVX512 & SIMD
	}
}

#undef SIMD

#pragma warning(pop)
