#pragma once
#include "Core.hpp"
#include "Ray.hpp"
#include "BaseHittable.hpp"

#include <memory>


namespace OWC
{
	class RTCamera
	{
	public:
		RTCamera() = delete;
		explicit RTCamera(const std::shared_ptr<BaseHittable>& hittables, std::vector<Colour>& pixels);

		OWC_FORCE_INLINE void SetRotate(const Vec3& eulerAngles) { m_Rotation = eulerAngles; m_PositionNeedsUpdating = true; }
		OWC_FORCE_INLINE void SetPosition(const Vec3& position) { m_Position = position; m_PositionNeedsUpdating = true; }
		OWC_FORCE_INLINE void SetScreenSize(const Vec2& size) { m_ScreenSize = size; m_PositionNeedsUpdating = true; }
		OWC_FORCE_INLINE void SetFOV(const float fov) { m_FOV = fov; m_PositionNeedsUpdating = true; }
		OWC_FORCE_INLINE void SetVup(const Vec3& vup) { m_Vup = vup; m_PositionNeedsUpdating = true; }
		OWC_FORCE_INLINE void SetFocalLength(const float focalLength) { m_FocalLength = focalLength; m_PositionNeedsUpdating = true; }

		OWC_FORCE_INLINE void SetNumberOfSamplesPerPass(const size_t samples) { m_NumberOfSamplesPerPass = samples; }

		void SingleThreadedRenderPass();
//		void MultiThreadedRenderPass();

	private:
		void CreateViewMatrix();
		Ray CreateRay(size_t i, size_t j) const;

		Colour RayColour(Ray ray);

	private:
		Vec3 m_Position{ 0.0f };
		Vec3 m_Rotation{ 0.0f };
		Vec3 m_Vup{ 0.0f, 1.0f, 0.0f };
		Vec3 m_PixelDeltaU{};
		Vec3 m_PixelDeltaV{};
		Point m_Pixel100Location{};
		Vec2 m_ScreenSize{};
		float m_FocalLength = 5.0f;
		float m_FOV = 30.0f;

		bool m_PositionNeedsUpdating = false;

		size_t m_NumberOfSamplesPerPass = 1;
		size_t m_MaxBounces = 5;

		std::vector<Colour>& m_Pixels;
		std::vector<Colour> m_BouncedColours;
		std::shared_ptr<BaseHittable> m_Hittables;
	};
}
