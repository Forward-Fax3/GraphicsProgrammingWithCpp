#pragma once
#include "Layer.hpp"
#include "BaseShader.hpp"
#include "Renderer.hpp"

#include <memory>


namespace OWC
{
	class TestLayer : public Layer
	{
	public:
		TestLayer();
		~TestLayer() override = default;
		TestLayer(const TestLayer&) = delete;
		TestLayer& operator=(const TestLayer&) = delete;
		TestLayer(TestLayer&&) = delete;
		TestLayer& operator=(TestLayer&&) = delete;

		void OnUpdate() override;
		void ImGuiRender() override;
		void OnEvent(class BaseEvent& event) override;

	private:
		void setupRenderPass();

	private:
		std::unique_ptr<Graphics::BaseShader> m_Shader = nullptr;
		std::shared_ptr<Graphics::RenderPassData> m_renderPass = nullptr;
	};
}
