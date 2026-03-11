#include "BasicScene.hpp"
#include "Sphere.hpp"
#include "Lambertian.hpp"


namespace OWC
{
	BasicScene::BasicScene()
	{
		m_Hittables = std::make_shared<Sphere>(Vec3(0.0f), 1.0f,
			std::make_shared<Lambertian>(Colour(1.0f, 0.0f, 0.0f, 1.0))
		);
	}

	void BasicScene::SetBaseCameraSettings(CameraRenderSettings& cameraSettings) const
	{
		cameraSettings.Position = Point(0.0f, 0.0f, -5.0f);
		cameraSettings.Rotation = Vec3(0.0f);
		cameraSettings.FOV = 45.0f;
		cameraSettings.FocalLength = 45.0f;
	}
}