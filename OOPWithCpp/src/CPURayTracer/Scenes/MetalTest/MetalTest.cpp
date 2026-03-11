#include "MetalTest.hpp"

#include "Sphere.hpp"
#include "SplitBVH.hpp"

#include "Lambertian.hpp"
#include "Metal.hpp"
#include "DefusedLight.hpp"


namespace OWC
{
	MetalTest::MetalTest()
	{
		m_SceneObjects = std::make_shared<Hitables>();
		m_SceneObjects->Reserve(8);
		m_SceneObjects->SetBackgroundFunction([](const Ray& ray)
			{
				constexpr f32 scale = 0.4f;
				f32 t = 0.5f * (ray.GetDirection().y + 1.0f);
				return (1.0f - t) + t * Colour(0.5f, 0.7f, 1.0f, 1.0f) * scale;
			});

		const Colour metalColour(0.8f, 0.2f, 0.1f, 1.0f);

		// Ground
		{
			auto groundMaterial = std::make_shared<Lambertian>(Colour(0.8f, 0.8f, 0.0f, 1.0f));
			auto groundSphere = std::make_shared<Sphere>(Point(0.0f, 100.5f, 0.0f), 100.0f, groundMaterial);
			m_SceneObjects->AddObject(groundSphere);
		}
		// the sun
		{
			auto sunMaterial = std::make_shared<DefusedLight>(Colour(1.0f, 1.0f, 1.0f, 1.0f), 5.0f);
			auto sunSphere = std::make_shared<Sphere>(Point(50.0f, -50.0f, -50.0f), 10.0f, sunMaterial);
			m_SceneObjects->AddObject(sunSphere);
		}

		// Metal spheres
		for (size_t i = 0; i < 6; i++)
		{
			auto if32 = static_cast<f32>(i);
			auto metalMaterial = std::make_shared<Metal>((i == 0) ? 0.0f : if32 / 5.0f , metalColour);
			auto sphere = std::make_shared<Sphere>(Point(-2.5 + if32, 0.0, 0.0), 0.5f, metalMaterial);
			m_SceneObjects->AddObject(sphere);
		}

		m_Hittable = std::make_shared<SplitBVH>(m_SceneObjects);
//		m_Hittable = m_SceneObjects;
	}

	void MetalTest::SetBaseCameraSettings(CameraRenderSettings& cameraSettings) const
	{
		cameraSettings.Position = Point(0.0f, 0.0f, -5.0f);
		cameraSettings.Rotation = Vec3(0.0f);
		cameraSettings.FOV = 50.0f;
		cameraSettings.FocalLength = 600.0f;
	}
}
