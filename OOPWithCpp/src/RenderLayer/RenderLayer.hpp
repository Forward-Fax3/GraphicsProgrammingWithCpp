#pragma once
#include "Layer.hpp"
#include "BaseShader.hpp"
#include "Renderer.hpp"
#include "UniformBuffer.hpp"
#include "InterLayerData.hpp"

#include <memory>
#include <glm/glm.hpp>


namespace OWC
{
	class RenderLayer : public Layer
	{
	public:
		RenderLayer() = delete;
		explicit RenderLayer(const std::shared_ptr<InterLayerData>& ILD);
		~RenderLayer() override = default;
		RenderLayer(const RenderLayer&) = delete;
		RenderLayer& operator=(const RenderLayer&) = delete;
		RenderLayer(RenderLayer&&) = delete;
		RenderLayer& operator=(RenderLayer&&) = delete;

		void OnUpdate() override;
		void ImGuiRender() override;
		void OnEvent(class BaseEvent& event) override;

	private:
		void SetupRenderPass();
		void SetupPipeline();

	private:
		std::unique_ptr<Graphics::BaseShader> m_Shader = nullptr;
		std::shared_ptr<Graphics::RenderPassData> m_renderPass = nullptr;
		std::shared_ptr<Graphics::UniformBuffer> m_UniformBuffer = nullptr;
		std::shared_ptr<Graphics::TextureBuffer> m_Image = nullptr;
		std::shared_ptr<InterLayerData> m_ILD = nullptr;
	};
}
