#pragma once
#include "Scene.hpp"
#include "Camera.hpp"

#include "Hittables.hpp"


namespace OWC
{
	class DuelGraySpheres : public BaseScene
	{
	public:
		DuelGraySpheres();
		~DuelGraySpheres() override = default;

		DuelGraySpheres(const DuelGraySpheres&) = delete;
		DuelGraySpheres& operator=(const DuelGraySpheres&) = delete;
		DuelGraySpheres(DuelGraySpheres&&) = delete;
		DuelGraySpheres& operator=(DuelGraySpheres&&) = delete;

		void SetBaseCameraSettings(CameraRenderSettings& cameraSettings) const override;

		const std::shared_ptr<BaseHitable>& GetHitable() override { return m_SceneObject; }

	private:
		std::shared_ptr<Hitables> m_SceneObjects = nullptr;
		std::shared_ptr<BaseHitable> m_SceneObject = nullptr;
	};
}
