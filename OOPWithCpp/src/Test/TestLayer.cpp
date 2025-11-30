#include "TestLayer.hpp"
#include "Log.hpp"
#include "LoadFile.hpp"
#include "VulkanCore.hpp"


namespace OWC
{
	TestLayer::TestLayer()
	{
		std::vector<Graphics::ShaderData> shaderDatas = {
			{
				.bytecode = LoadFileToBytecode<uint32_t>("../ShaderSrc/test.vert.spv"),
				.type = Graphics::ShaderData::ShaderType::Vertex,
				.language = Graphics::ShaderData::ShaderLanguage::SPIRV
			},
			{
				.bytecode = LoadFileToBytecode<uint32_t>("../ShaderSrc/test.frag.spv"),
				.type = Graphics::ShaderData::ShaderType::Fragment,
				.language = Graphics::ShaderData::ShaderLanguage::SPIRV
			}
		};

		m_Shader = Graphics::BaseShader::CreateShader(shaderDatas);

		m_CommandBuffer = Graphics::VulkanCore::GetInstance().GetGraphicsCommandBuffer();
	}

	TestLayer::~TestLayer()
	{
		Graphics::VulkanCore::GetInstance().GetDevice().freeCommandBuffers(
			Graphics::VulkanCore::GetInstance().GetGraphicsCommandPool(),
			m_CommandBuffer);
	}

	void TestLayer::OnUpdate()
	{
		// TODO: Move this to Renderer instead of it being done it vkCore
		const Graphics::VulkanCore& vkCore = Graphics::VulkanCore::GetConstInstance();
		vkCore.BeginRenderPass(m_CommandBuffer, m_Shader->GetPipeline());
		m_CommandBuffer.draw(3, 1, 0, 0);
		vkCore.EndRenderPass(m_CommandBuffer);
		vkCore.SubmitGraphicsCommandBuffer(m_CommandBuffer);
	}

	void TestLayer::ImGuiRender()
	{
	}
	void TestLayer::OnEvent(class BaseEvent&)
	{
	}
}
