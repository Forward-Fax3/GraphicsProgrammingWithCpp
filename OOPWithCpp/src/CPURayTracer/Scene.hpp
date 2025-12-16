#pragma once
#include "core.hpp"

#include <vector>
#include <memory>
#include <glm/vec4.hpp>


namespace OWC
{
	enum class Scene : uint8_t
	{ // TODO: Add more scenes
		Basic , // A basic scene with just a sphere
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

		virtual bool RenderNextPass() = 0;

		static std::unique_ptr<BaseScene> CreateScene(Scene scene, std::vector<Vec4>& frameBuffer);

	protected:
		explicit BaseScene(std::vector<Vec4>& frameBuffer)
			: m_FrameBuffer(frameBuffer) {}

		std::vector<Vec4>& GetFrameBuffer() { return m_FrameBuffer; }

	private:
		std::vector<Vec4>& m_FrameBuffer;
	};
}
