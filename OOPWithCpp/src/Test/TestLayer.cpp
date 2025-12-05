#include "TestLayer.hpp"
#include "Application.hpp"
#include "LoadFile.hpp"
#include "Renderer.hpp"

#include "BaseEvent.hpp"
#include "WindowMinimizeEvent.hpp"
#include "WindowRestoreEvent.hpp"

#include <array>


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

		std::array<std::string_view, 1> waitSemaphorenames = { "ImageReady" };
		std::array<std::string_view, 1> signalSemaphoreNames = { "TestLayer" };

		Renderer::RestartRenderPass(m_renderPass);
		Renderer::SubmitRenderPass(m_renderPass, waitSemaphorenames, signalSemaphoreNames);
	}

	void TestLayer::ImGuiRender()
	{ // not needed
	}

	void TestLayer::OnEvent(class BaseEvent& e)
	{
		EventDispatcher dispatcher(e);

		dispatcher.Dispatch<WindowRestore>([this](const WindowRestore& /*event*/) {
			this->SetActive(true);
			return false;
			});

		if (!IsActive())
			return;

		dispatcher.Dispatch<WindowMinimize>([this](const WindowMinimize& /*event*/) {
			this->SetActive(false);
			return false;
			});
	}

	void TestLayer::setupRenderPass()
	{
		using namespace OWC::Graphics;
		m_renderPass = Renderer::BeginPass();
		Renderer::PipelineBind(m_renderPass, *m_Shader);
		Renderer::Draw(m_renderPass, 6);
		Renderer::EndPass(m_renderPass);
	}
}
