#pragma once
#include "Layer.hpp"
#include "BaseShader.hpp"

#include <memory>


namespace OWC
{
	class TestLayer : public Layer
	{
	public:
		TestLayer();
		void OnUpdate() override;
		void ImGuiRender() override;
		void OnEvent(class BaseEvent& event) override;

	private:
		std::unique_ptr<Graphics::BaseShader> m_Shader;
	};
}
