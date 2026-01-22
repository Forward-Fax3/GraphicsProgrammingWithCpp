#include "Core.hpp"
#include "Application.hpp"
#include "DuelWhiteSpheres.hpp"

#include "Hittables.hpp"
#include "SplitBVH.hpp"
#include "Sphere.hpp"

#include "Lambertian.hpp"
#include "DefusedLight.hpp"

#include "BaseEvent.hpp"
#include "WindowResize.hpp"

#include <imgui.h>

#include <glm/gtc/type_ptr.hpp>


namespace OWC
{
	DuelGraySpheres::DuelGraySpheres()
	{
		m_SceneObjects = std::make_shared<Hitables>();
		m_SceneObjects->Reserve(3);
		m_SceneObjects->SetBackgroundFunction([](const Ray& ray)
			{
				constexpr f32 scale = 0.4f;
				f32 t = 0.5f * (ray.GetDirection().y + 1.0f);
				return (1.0f - t) + t * Colour(0.5f, 0.7f, 1.0f, 1.0f) * scale;
			});

		auto gray = std::make_shared<Lambertian>(Colour(0.5f));

		// small sphere
		{
			auto smallSphere = std::make_shared<Sphere>(Vec3(0.0f), 1.0f, gray);
			m_SceneObjects->AddObject(smallSphere);
		}
		// big sphere
		{
			auto bigSphere = std::make_shared<Sphere>(Vec3(0.0f, 51.0f, 0.0f), 50.0f, gray);
			m_SceneObjects->AddObject(bigSphere);
		}
		// sun
		{
			auto sunMaterial = std::make_shared<DefusedLight>(Colour(1.0f), 50.0f);
			auto sunSphere = std::make_shared<Sphere>(Point(50.0f, -50.0f, -50.0f), 10.0f, sunMaterial);
			m_SceneObjects->AddObject(sunSphere);
		}

//		m_SceneObject = std::make_shared<SplitBVH>(m_SceneObjects);
		m_SceneObject = m_SceneObjects;
	}

	void DuelGraySpheres::SetBaseCameraSettings(CameraRenderSettings& cameraSettings) const
	{
		cameraSettings.Position = Point(0.0f, 0.0f, -5.0f);
		cameraSettings.Rotation = Vec3(0.0f);
		cameraSettings.FOV = 50.0f;
		cameraSettings.FocalLength = 600.0f;
	}
}
