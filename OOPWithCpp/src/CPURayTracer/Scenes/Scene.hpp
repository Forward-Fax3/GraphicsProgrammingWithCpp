#pragma once
#include "Core.hpp"
#include "Camera.hpp"

#include <memory>


namespace OWC
{
	enum class Scene : u8
	{ // TODO: Add more scenes
		Basic , // A basic scene with just a sphere
//		RandTest, // A scene that fills the framebuffer with random colors
		DuelGreySpheres, // A scene with two gray spheres
		DielectricTest,
		MetalTest,
		EarthScene,
		Book1FinalRender
	};

	class BaseScene
	{
	public:
		BaseScene() = default;
		virtual ~BaseScene() = default;

		BaseScene(const BaseScene&) = delete;
		BaseScene& operator=(const BaseScene&) = delete;
		BaseScene(BaseScene&&) = delete;
		BaseScene& operator=(BaseScene&&) = delete;

		virtual void SetBaseCameraSettings(CameraRenderSettings& cameraSettings) const = 0;

		virtual const std::shared_ptr<BaseHitable>& GetHitable() = 0;

		virtual void OnImGuiRender() { /* default empty implementation */ }

		static std::unique_ptr<BaseScene> CreateScene(Scene scene);
	};
}
