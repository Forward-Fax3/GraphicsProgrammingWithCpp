#include "MetalTest.hpp"

#include "Sphere.hpp"
#include "Lambertian.hpp"
#include "Metal.hpp"
#include "DefusedLight.hpp"


namespace OWC
{
	MetalTest::MetalTest()
	{
		m_SceneObjects = std::make_shared<Hitables>();
		m_Hittable = m_SceneObjects;
		m_SceneObjects->Reserve(8);
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
	}

	void MetalTest::SetBaseCameraSettings(CameraRenderSettings& cameraSettings) const
	{
		cameraSettings.Position = Point(0.0f, 0.0f, -5.0f);
		cameraSettings.Rotation = Vec3(0.0f);
		cameraSettings.FOV = 50.0f;
		cameraSettings.FocalLength = 600.0f;
	}
}
