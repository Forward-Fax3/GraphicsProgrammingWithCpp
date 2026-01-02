#include "Application.hpp"
#include "Camera.hpp"
#include "Ray.hpp"
#include "OWCRand.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

#include <ranges>


namespace OWC
{
	RenderPassReturnData RTCamera::SingleThreadedRenderPass(const std::shared_ptr<BaseHittable>& hittables)
	{
		Vec2us screenSize(m_Settings.ScreenSize);

		for (uSize i = 0; i != screenSize.y; i++)
			for (uSize j = 0; j != screenSize.x; j++)
				for (uSize k = 0; k != m_Settings.NumberOfSamplesPerPass; k++)
				{
					Ray ray = CreateRay(i, j);
					m_Pixels[(i * screenSize.x) + j] += RayColour(ray, hittables);
				}

		return true;
	}

	void RTCamera::UpdateCameraSettings()
	{
		m_BouncedColours.resize(m_Settings.MaxBounces + 1); // +1 for lost ray colour

		Vec3 rotationInRadians = glm::radians(m_Settings.Rotation);
		Mat4 rotationMatrix = glm::eulerAngleYXZ(rotationInRadians.y, rotationInRadians.x, rotationInRadians.z);
		Vec3 forward = glm::normalize(Vec3(rotationMatrix * Vec4(0.0f, 0.0f, -1.0f, 0.0f)));
		Vec3 right = glm::normalize(Vec3(rotationMatrix * Vec4(1.0f, 0.0, 0.0, 0.0)));
		Vec3 up = glm::normalize(Vec3(rotationMatrix * Vec4(0.0f, -1.0, 0.0, 0.0)));

		f32 aspectRatio = m_Settings.ScreenSize.x / m_Settings.ScreenSize.y;
		f32 viewportHeight = 2.0f * m_Settings.FocalLength * glm::tan(glm::radians(m_Settings.FOV) * 0.5f);
		f32 viewportWidth = aspectRatio * viewportHeight;

		Point viewportU = viewportWidth * right;
		Point viewportV = viewportHeight * -up;

		m_PixelDeltaU = viewportU / m_Settings.ScreenSize.x;
		m_PixelDeltaV = viewportV / m_Settings.ScreenSize.y;

		Point viewportUpperLeft = m_Settings.Position - (m_Settings.FocalLength * forward) - 0.5f * (viewportU + viewportV);
		m_Pixel100Location = viewportUpperLeft + 0.5f * (m_PixelDeltaU + m_PixelDeltaV);
	}

	Ray RTCamera::CreateRay(uSize i, uSize j) const
	{
		Vec2 randomOffset = Rand::LinearFastRandVec2(Vec2(0.0f), Vec2(1.0f)) + Vec2(j, i);
		Vec3 rayDirection = m_Pixel100Location +
			(randomOffset.x * m_PixelDeltaU) +
			(randomOffset.y * m_PixelDeltaV);

		return Ray(m_Settings.Position, rayDirection);
	}

	Colour RTCamera::RayColour(Ray ray, const std::shared_ptr<BaseHittable>& hittables)
	{
		bool missed = false;
		uSize i = 0;

		for (; i != m_Settings.MaxBounces; i++)
		{
			Interval tRange(0.001f, std::numeric_limits<f32>::max());
			HitData hitData = m_Hittables->IsHit(ray, tRange);
			if (!hitData.hasHit)
			{
				f32 t = 0.5f * (ray.GetDirection().y + 1.0f);
				m_BouncedColours[i] = (1.0f - t) + t * Colour(0.5f, 0.7f, 1.0f, 1.0f);
				missed = true;
				break;
			}

			m_BouncedColours[i] = hitData.material->Albedo(hitData);
			hitData.material->Scatter(ray, hitData);
		}

		if (i == m_Settings.MaxBounces)
		{
			m_BouncedColours[m_Settings.MaxBounces] = Colour(0.0f);
			missed = true;
		}

		if (missed)
			i++;

		Colour finalColour(1.0f);
		while (i != 0)
		{
			i--;
			finalColour *= m_BouncedColours[i];
		}

		return finalColour;
	}
}
