#include "EarthScene.hpp"

#include "Sphere.hpp"

#include "ImageTexture.hpp"

#include "Lambertian.hpp"
#include "DefusedLight.hpp"


namespace OWC
{
	EarthScene::EarthScene()
	{
		m_SceneObjects = std::make_shared<Hitables>();
		m_Hittable = m_SceneObjects;
		m_SceneObjects->Reserve(2);

		// Load Earth sphere
		{
			auto earthTexture = std::make_shared<ImageTexture>("../Images/EarthMap.jpg");
			auto earthMaterial = std::make_shared<Lambertian>(earthTexture);
			m_SceneObjects->AddObject(std::make_shared<Sphere>(Vec3(0.0f, 0.0f, 0.0f), 1.0f, earthMaterial));
		}
		// sun
		{
			auto sunMaterial = std::make_shared<DefusedLight>(Colour(1.0f), 5.0f);
			auto sunSphere = std::make_shared<Sphere>(Point(50.0f, -50.0f, -50.0f), 10.0f, sunMaterial);
			m_SceneObjects->AddObject(sunSphere);
		}
	}

	void EarthScene::SetBaseCameraSettings(CameraRenderSettings& cameraSettings) const
	{
		cameraSettings.Position = Point(0.0f, 0.0f, -5.0f);
		cameraSettings.Rotation = Vec3(0.0f);
		cameraSettings.FOV = 50.0f;
		cameraSettings.FocalLength = 600.0f;
	}
}
