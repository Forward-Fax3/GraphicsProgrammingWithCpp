#pragma once
#include "Scene.hpp"
#include "BaseHittable.hpp"
#include "Camera.hpp"


namespace OWC
{
	class BasicScene : public BaseScene
	{
	public:
		BasicScene();
		~BasicScene() override = default;

		BasicScene(const BasicScene&) = delete;
		BasicScene& operator=(const BasicScene&) = delete;
		BasicScene(BasicScene&&) = delete;
		BasicScene& operator=(BasicScene&&) = delete;

		void SetBaseCameraSettings(CameraRenderSettings& cameraSettings) const override;

		const std::shared_ptr<BaseHitable>& GetHitable() override { return m_Hittables; }

	private:
		std::shared_ptr<BaseHitable> m_Hittables;
	};
}
