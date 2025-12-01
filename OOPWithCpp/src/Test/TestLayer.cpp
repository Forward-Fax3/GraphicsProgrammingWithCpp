#include "TestLayer.hpp"
#include "Application.hpp"
#include "Log.hpp"
#include "LoadFile.hpp"
#include "Renderer.hpp"


namespace OWC
{
	TestLayer::TestLayer()
	{
		using namespace OWC::Graphics;

		std::vector<ShaderData> shaderDatas = {
			{
				.bytecode = LoadFileToBytecode<uint32_t>("../ShaderSrc/test.vert.spv"),
				.type = ShaderData::ShaderType::Vertex,
				.language = ShaderData::ShaderLanguage::SPIRV
			},
			{
				.bytecode = LoadFileToBytecode<uint32_t>("../ShaderSrc/test.frag.spv"),
				.type = ShaderData::ShaderType::Fragment,
				.language = ShaderData::ShaderLanguage::SPIRV
			}
		};

		m_Shader = BaseShader::CreateShader(shaderDatas);

		setupRenderPass();
	}

	void TestLayer::OnUpdate()
	{
		using namespace OWC::Graphics;
		if (Application::GetConstInstance().GetWindow().GetGraphicsContext().RenderPassNeedsRecreating())
			setupRenderPass();

		Renderer::RestartRenderPass(m_renderPass);
		Renderer::SubmitRenderPass(m_renderPass);
	}

	void TestLayer::ImGuiRender()
	{ // ImGui not implemented yet
	}

	void TestLayer::OnEvent(class BaseEvent&)
	{ // not needed for this test layer
	}

	void TestLayer::setupRenderPass()
	{
		using namespace OWC::Graphics;
		m_renderPass = (Renderer::BeginPass(RenderPassType::Graphics));
		Renderer::PipelineBind(m_renderPass, *m_Shader);
		Renderer::Draw(m_renderPass, 6);
		Renderer::EndPass(m_renderPass);
	}
}
