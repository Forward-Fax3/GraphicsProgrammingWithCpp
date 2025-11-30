#pragma once
#include "Layer.hpp"
#include "BaseShader.hpp"
#include <vulkan/vulkan.hpp>

#include <memory>


namespace OWC
{
	class TestLayer : public Layer
	{
	public:
		TestLayer();
		~TestLayer() override;
		TestLayer(const TestLayer&) = delete;
		TestLayer& operator=(const TestLayer&) = delete;
		TestLayer(TestLayer&&) = delete;
		TestLayer& operator=(TestLayer&&) = delete;

		void OnUpdate() override;
		void ImGuiRender() override;
		void OnEvent(class BaseEvent& event) override;

	private:
		std::unique_ptr<Graphics::BaseShader> m_Shader = nullptr;
		vk::CommandBuffer m_CommandBuffer = vk::CommandBuffer();
	};
}
