#include "Application.hpp"
#include "Camera.hpp"
#include "Ray.hpp"
#include "OWCRand.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

#include <ranges>


namespace OWC
{
	RTCamera::RTCamera(const std::shared_ptr<BaseHittable>& hittables, std::vector<Colour>& pixels)
		: m_Pixels(pixels), m_Hittables(hittables)
	{
		const auto& app = Application::GetConstInstance();
		SetScreenSize(Vec2(static_cast<float>(app.GetWindowWidth()), static_cast<float>(app.GetWindowHeight())));
	}

	void RTCamera::SingleThreadedRenderPass()
	{
		if (m_PositionNeedsUpdating)
			CreateViewMatrix();

		glm::vec<2, size_t> screenSize(m_ScreenSize);

		for (size_t i = 0; i != screenSize.y; i++)
			for (size_t j = 0; j != screenSize.x; j++)
				for (size_t k = 0; k != m_NumberOfSamplesPerPass; k++)
				{
					Ray ray = CreateRay(i, j);
					m_Pixels[(i * screenSize.x) + j] += RayColour(ray);
				}
	}

	void RTCamera::CreateViewMatrix()
	{
		m_BouncedColours.resize(m_MaxBounces + 1); // +1 for lost ray colour

		Vec3 rotationInRadians = glm::radians(m_Rotation);
		Mat4 rotationMatrix = glm::eulerAngleYXZ(rotationInRadians.y, rotationInRadians.x, rotationInRadians.z);
		Vec3 forward = glm::normalize(Vec3(rotationMatrix * Vec4(0.0f, 0.0f, 1.0f, 0.0f)));
		Vec3 right = glm::normalize(Vec3(rotationMatrix * Vec4(1.0f, 0.0, 0.0, 0.0)));
		Vec3 up = glm::normalize(Vec3(rotationMatrix * Vec4(0.0f, -1.0, 0.0, 0.0)));

		float aspectRatio = m_ScreenSize.x / m_ScreenSize.y;
		float viewportHeight = 2.0f * m_FocalLength * glm::tan(glm::radians(m_FOV) * 0.5f);
		float viewportWidth = aspectRatio * viewportHeight;

		Point viewportU = viewportWidth * right;
		Point viewportV = viewportHeight * -up;

		m_PixelDeltaU = viewportU / m_ScreenSize.x;
		m_PixelDeltaV = viewportV / m_ScreenSize.y;

		Point viewportUpperLeft = m_Position - (m_FocalLength * forward) - 0.5f * (viewportU + viewportV);
		m_Pixel100Location = viewportUpperLeft + 0.5f * (m_PixelDeltaU + m_PixelDeltaV);
		
		m_PositionNeedsUpdating = false;
	}

	Ray RTCamera::CreateRay(size_t i, size_t j) const
	{
		Vec2 randomOffset = Rand::LinearFastRandVec2(Vec2(0.0f), Vec2(1.0f)) + Vec2(j, i);
		Vec3 rayDirection = m_Pixel100Location +
			(randomOffset.x * m_PixelDeltaU) +
			(randomOffset.y * m_PixelDeltaV);

		return Ray(m_Position, rayDirection);
	}

	Colour RTCamera::RayColour(Ray ray)
	{
		bool missed = false;
		size_t i = 0;

		for (; i != m_MaxBounces; i++)
		{
			HitData hitData = m_Hittables->IsHit(ray);
			if (!hitData.hasHit)
			{
				float t = 0.5f * (ray.GetDirection().y + 1.0f);
				m_BouncedColours[i] = (1.0f - t) + t * Colour(0.5f, 0.7f, 1.0f, 1.0f);
				missed = true;
				break;
			}

			m_BouncedColours[i] = hitData.material->Albedo(hitData);
			hitData.material->Scatter(ray, hitData);
		}

		if (i == m_MaxBounces)
		{
			m_BouncedColours[m_MaxBounces] = Colour(0.0f);
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
