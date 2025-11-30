#include "TestLayer.hpp"
#include "Log.hpp"
#include "LoadFile.hpp"


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
	}

	void TestLayer::OnUpdate()
	{
	}

	void TestLayer::ImGuiRender()
	{
	}
	void TestLayer::OnEvent(class BaseEvent&)
	{
	}
}
