//
// Created by forwardfax3 on 15/03/2026.
//

#include "Core.hpp"
#include "GPURayTracerRenderer.hpp"
#include "LoadFile.hpp"
#include "Renderer.hpp"

#include "BaseEvent.hpp"
#include "WindowMinimizeEvent.hpp"
#include "WindowRestoreEvent.hpp"
#include "WindowResize.hpp"

#include "SponzaPalace.hpp"

#include <array>

#include <glm/gtc/type_ptr.hpp>


namespace OWC
{
	//static bool operator>(const Vec2u& lhs, u32 rhs)
	//{
	//	return (lhs.x > rhs) && (lhs.y > rhs);
	//}

	GPURayTracerRenderer::GPURayTracerRenderer()
	{
		SetupPipeline();

		m_Scene = std::make_shared<SponzaPalace>();
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

		std::vector<BindingDescription> rayGenBindingDiscriptions = {
			{
				.descriptorCount = 1,
				.binding = 1,
				.descriptorType = DescriptorType::StorageImage,
				.stageFlags = ShaderType::RayGen
			},
			{
				.descriptorCount = 1,
				.binding = 2,
				.descriptorType = DescriptorType::TLAS,
				.stageFlags = (ShaderType::RayGen | ShaderType::RayClosestHit)
			},
			{
				.descriptorCount = 1,
				.binding = 0,
				.descriptorType = DescriptorType::UniformBuffer,
				.stageFlags = (ShaderType::RayGen)
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
				.binding = 3,
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
				.descriptorType = rayGenBindingDiscriptions,
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
		// m_Shader->BindUniform(0, m_UniformBuffer);
		// m_Shader->BindDynamicTexture(1, m_Image);
	}
}
