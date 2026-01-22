#pragma once
#include "Scene.hpp"
#include "Hittables.hpp"


namespace OWC
{
	class DielectricTest : public BaseScene
	{
	public:
		DielectricTest();
		~DielectricTest() override = default;
		DielectricTest(const DielectricTest&) = delete;
		DielectricTest& operator=(const DielectricTest&) = delete;
		DielectricTest(DielectricTest&&) = delete;
		DielectricTest& operator=(DielectricTest&&) = delete;

		void SetBaseCameraSettings(CameraRenderSettings& cameraSettings) const override;
		const std::shared_ptr<BaseHitable>& GetHitable() override { return m_Hittable; }

	private:
		std::shared_ptr<Hitables> m_SceneObjects;
		std::shared_ptr<BaseHitable> m_Hittable;
	};
}
