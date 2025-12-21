#pragma once
#include "Core.hpp"
#include "BaseEvent.hpp"

#include <vector>
#include <memory>
#include <glm/vec4.hpp>


namespace OWC
{
	enum class Scene : uint8_t
	{ // TODO: Add more scenes
		Basic , // A basic scene with just a sphere
		RandTest, // A scene that fills the framebuffer with random colors
		DuelGraySpheres // A scene with two gray spheres
	};

	struct RenderPassReturnData
	{
		bool frameBufferUpdated = false;
		bool frameNeedsReset = false;
	};

	class BaseScene
	{
	public:
		BaseScene() = delete;
		virtual ~BaseScene() = default;

		BaseScene(const BaseScene&) = delete;
		BaseScene& operator=(const BaseScene&) = delete;
		BaseScene(BaseScene&&) = delete;
		BaseScene& operator=(BaseScene&&) = delete;

		virtual RenderPassReturnData RenderNextPass() = 0;

		virtual void OnImGuiRender() { /* default empty implementation */ }
		virtual bool OnEvent(BaseEvent&) { return false; /* default empty implementation */ }

		static std::unique_ptr<BaseScene> CreateScene(Scene scene, std::vector<Vec4>& frameBuffer);

	protected:
		explicit BaseScene(std::vector<Vec4>& frameBuffer)
			: m_FrameBuffer(frameBuffer) {}

		std::vector<Vec4>& GetFrameBuffer() { return m_FrameBuffer; }

	private:
		std::vector<Vec4>& m_FrameBuffer;
	};
}
