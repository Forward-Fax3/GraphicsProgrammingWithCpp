//
// Created by forwardfax3 on 15/03/2026.
//

#include "Core.hpp"
#include "GPURayTracerRenderer.hpp"
#include "LoadFile.hpp"
#include "Renderer.hpp"
#include "Application.hpp"

#include "BaseEvent.hpp"
#include "WindowMinimizeEvent.hpp"
#include "WindowRestoreEvent.hpp"
#include "WindowResize.hpp"

#include "SponzaPalace.hpp"

#include <array>

#include <glm/gtc/type_ptr.hpp>

#include "Application.hpp"


namespace OWC
{
	//static bool operator>(const Vec2u& lhs, u32 rhs)
	//{
	//	return (lhs.x > rhs) && (lhs.y > rhs);
	//}

	GPURayTracerRenderer::GPURayTracerRenderer()
	{
		auto& app = Application::GetConstInstance();
		m_Scene = std::make_shared<SponzaPalace>();
		m_RenderTarget = Graphics::TextureBuffer::CreateTextureBuffer(app.GetPixelWidth(), app.GetPixelHeight());

		SetupPipeline();
	}

	void GPURayTracerRenderer::OnUpdate()
	{}

	void GPURayTracerRenderer::OnEvent(BaseEvent& e)
	{
		EventDispatcher dispatcher(e);

		dispatcher.Dispatch<WindowRestore>([this](const WindowRestore& /*event*/) {
			this->SetupRenderPass();
			this->SetActiveState(true);
			return false;
			});

		if (!IsActive())
			return;

		dispatcher.Dispatch<WindowMinimize>([this](const WindowMinimize& /*event*/) {
			this->SetActiveState(false);
			return false;
			});

		dispatcher.Dispatch<WindowResize>([this](const WindowResize& /*event*/) {
			this->SetupPipeline();
			this->SetupRenderPass();
			return false;
			});
	}

	void GPURayTracerRenderer::SetupRenderPass()
	{
	}

	void GPURayTracerRenderer::SetupPipeline()
	{
		using namespace OWC::Graphics;

		std::vector<BindingDescription> rayGenBindingDescriptions = {
			{
				.descriptorCount = 1,
				.binding = 0,
				.descriptorType = DescriptorType::StorageImage,
				.stageFlags = ShaderType::RayGen
			},
			{
				.descriptorCount = 1,
				.binding = 1,
				.descriptorType = DescriptorType::TLAS,
				.stageFlags = (ShaderType::RayGen | ShaderType::RayClosestHit)
			}
		};

		std::vector<BindingDescription> closestHitBindingDiscriptions = {
			/*{
				.descriptorCount = 1,
				.binding = 1,
				.descriptorType = DescriptorType::RTblas,
				.stageFlags = (ShaderType::RayGen | ShaderType::RayClosestHit)
			},*/
			{
				.descriptorCount = 1,
				.binding = 2,
				.descriptorType = DescriptorType::StorageBuffer,
				.stageFlags = (ShaderType::RayGen | ShaderType::RayClosestHit)
			}
		};

		auto shaderSrc = LoadFileToBytecode<u32>("../ShaderSrc/GPURayTracerShaders/test.spv");

		std::vector<ShaderData> shaderDatas = {
			{
				.bytecode = shaderSrc,
				.type = ShaderType::RayGen,
				.language = ShaderData::ShaderLanguage::SPIRV,
				.descriptorType = rayGenBindingDescriptions,
				.entryPoint = "rayGenShader",
			},
			{
				.bytecode = shaderSrc,
				.type = ShaderType::RayClosestHit,
				.language = ShaderData::ShaderLanguage::SPIRV,
				.descriptorType = closestHitBindingDiscriptions,
				.entryPoint = "closestHitShader",
			},
			{
				.bytecode = shaderSrc,
				.type = ShaderType::RayMiss,
				.language = ShaderData::ShaderLanguage::SPIRV,
				.descriptorType = {},
				.entryPoint = "missShader",
			}
		};

		m_Shader = BaseShader::CreateRTShader(shaderDatas);
		m_Shader->BindTexture(0, m_RenderTarget);
		m_Shader->BindTLAS(1, m_Scene->GetTLAS());
	}
}
