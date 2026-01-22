#pragma once
#include "Core.hpp"

#include "Scene.hpp"

#include "BaseHittable.hpp"
#include "Hittables.hpp"

#include <memory>


namespace OWC
{
	class MetalTest : public BaseScene
	{
	public:
		MetalTest();
		~MetalTest() override = default;

		MetalTest(MetalTest&) = delete;
		MetalTest operator=(MetalTest&) = delete;
		MetalTest(MetalTest&&) = delete;
		MetalTest operator=(MetalTest&&) = delete;

		void SetBaseCameraSettings(CameraRenderSettings& cameraSettings) const override;
		const std::shared_ptr<BaseHitable>& GetHitable() override { return m_Hittable; }

	private:
		std::shared_ptr<Hitables> m_SceneObjects;
		std::shared_ptr<BaseHitable> m_Hittable;
	};
}
