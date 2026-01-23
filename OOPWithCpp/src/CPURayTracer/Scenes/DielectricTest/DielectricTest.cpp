#include "DielectricTest.hpp"

#include "Sphere.hpp"
#include "SplitBVH.hpp"

#include "Lambertian.hpp"
#include "Dielectric.hpp"
#include "DefusedLight.hpp"


namespace OWC
{
	DielectricTest::DielectricTest()
	{
		m_SceneObjects = std::make_shared<Hitables>();
		m_SceneObjects->Reserve(8);
		m_SceneObjects->SetBackgroundFunction([this](const Ray& ray)
			{
				constexpr f32 tScale = 3.0f / 4.0f;
				return this->m_SceneObjects->BaseHitable::BackgroundColour(ray) * tScale;
			});

		// Ground
		{
			auto groundMaterial = std::make_shared<Lambertian>(Colour(0.8f, 0.8f, 0.0f, 1.0f));
			auto groundSphere = std::make_shared<Sphere>(Point(0.0f, 101.0f, 0.0f), 100.0f, groundMaterial);
			m_SceneObjects->AddObject(groundSphere);
		}
		// the sun
		{
			auto sunMaterial = std::make_shared<DefusedLight>(Colour(1.0f, 1.0f, 1.0f, 1.0f), 25.0f);
			auto sunSphere = std::make_shared<Sphere>(Point(50.0f, -50.0f, -50.0f), 10.0f, sunMaterial);
			m_SceneObjects->AddObject(sunSphere);
		}
		// Center outer sphere
		{
			auto centerMaterial = std::make_shared<Dielectric>(1.5f);
			auto centerSphere = std::make_shared<Sphere>(Point(0.0f, 0.0f, 0.0f), 1.0f, centerMaterial);
			m_SceneObjects->AddObject(centerSphere);
		}
		// center inner sphere (hollow)
		{
			auto centerInnerMaterial = std::make_shared<Dielectric>(1.0f / 1.5f);
			auto centerInnerSphere = std::make_shared<Sphere>(Point(0.0f, 0.0f, 0.0f), 0.9f, centerInnerMaterial);
			m_SceneObjects->AddObject(centerInnerSphere);
		}
		// Left sphere
		{
			auto leftMaterial = std::make_shared<Dielectric>(1.0f / 1.5f);
			auto leftSphere = std::make_shared<Sphere>(Point(-4.0f, -1.0f, 0.0f), 1.0f, leftMaterial);
			m_SceneObjects->AddObject(leftSphere);
		}
		// Right sphere
		{
			auto rightMaterial = std::make_shared<Dielectric>(1.0f / 1.5f);
			auto rightSphere = std::make_shared<Sphere>(Point(4.0f, -1.0f, 0.0f), 1.0f, rightMaterial);
			m_SceneObjects->AddObject(rightSphere);
		}
		// back sphere
		{
			auto backMaterial = std::make_shared<Dielectric>(1.5f);
			auto backSphere = std::make_shared<Sphere>(Point(0.0f, -2.0f, 4.0f), 1.0f, backMaterial);
			m_SceneObjects->AddObject(backSphere);
		}
		// back lower lambertian sphere
		{
			auto backLowerMaterial = std::make_shared<Lambertian>(Colour(0.8f, 0.3f, 0.3f, 1.0f));
			auto backLowerSphere = std::make_shared<Sphere>(Point(0.0f, 0.0f, 4.0f), 1.0f, backLowerMaterial);
			m_SceneObjects->AddObject(backLowerSphere);
		}

		m_Hittable = std::make_shared<SplitBVH>(m_SceneObjects);
//		m_Hittable = m_SceneObjects;
	}

	void DielectricTest::SetBaseCameraSettings(CameraRenderSettings& cameraSettings) const
	{
		cameraSettings.Position = Point(0.0f, 0.0f, -5.0f);
		cameraSettings.Rotation = Vec3(0.0f);
		cameraSettings.FOV = 50.0f;
		cameraSettings.FocalLength = 600.0f;
	}
}
