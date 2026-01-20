#pragma once
#include "Scene.hpp"

#include "Hittables.hpp"

#include "memory"


namespace OWC
{
	class EarthScene : public BaseScene
	{
	public:
		EarthScene();
		~EarthScene() override = default;

		EarthScene(const EarthScene&) = delete;
		EarthScene& operator=(const EarthScene&) = delete;
		EarthScene(EarthScene&&) = delete;
		EarthScene& operator=(EarthScene&&) = delete;

		void SetBaseCameraSettings(CameraRenderSettings& cameraSettings) const override;
		const std::shared_ptr<BaseHittable>& GetHitable() override { return m_Hittable; }

	private:
		std::shared_ptr<Hitables> m_SceneObjects;
		std::shared_ptr<BaseHittable> m_Hittable;
	};
}
