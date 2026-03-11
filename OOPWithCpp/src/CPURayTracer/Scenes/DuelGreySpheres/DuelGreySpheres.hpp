#pragma once
#include "Scene.hpp"
#include "Camera.hpp"

#include "Hittables.hpp"


namespace OWC
{
	class DuelGreySpheres : public BaseScene
	{
	public:
		DuelGreySpheres();
		~DuelGreySpheres() override = default;

		DuelGreySpheres(const DuelGreySpheres&) = delete;
		DuelGreySpheres& operator=(const DuelGreySpheres&) = delete;
		DuelGreySpheres(DuelGreySpheres&&) = delete;
		DuelGreySpheres& operator=(DuelGreySpheres&&) = delete;

		void SetBaseCameraSettings(CameraRenderSettings& cameraSettings) const override;

		const std::shared_ptr<BaseHitable>& GetHitable() override { return m_SceneObject; }

	private:
		std::shared_ptr<Hitables> m_SceneObjects = nullptr;
		std::shared_ptr<BaseHitable> m_SceneObject = nullptr;
	};
}
