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
		m_UniformBuffer = Graphics::UniformBuffer::CreateUniformBuffer(sizeof(UniformBufferObject));

		SetupPipeline();
		SetupRenderPass();
	}

	void GPURayTracerRenderer::OnUpdate()
	{
		using namespace OWC::Graphics;
		std::array<std::string_view, 1> RayTracerSignalSemaphoreNames = { "RayTracerDone" };
		std::array<std::string_view, 2> displayWaitSemaphoreNames = { "ImageReady", "RayTracerDone" };

		UniformBufferObject ubo{
			.divider = 1.0f / static_cast<f32>((m_ILD->numberOfSamples - 1 == 0) ? 1 : m_ILD->numberOfSamples - 1), // avoid division by zero
			.invGammaValue = m_ILD->invGammaValue
		};

		m_UniformBuffer->UpdateBufferData(std::as_bytes(std::span<const UniformBufferObject>(&ubo, 1)));

		Renderer::SubmitRenderPass(m_RayTracingRenderPass, {}, RayTracerSignalSemaphoreNames);
		Renderer::SubmitRenderPass(m_DisplayRenderPass, displayWaitSemaphoreNames, {});
	}

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
		struct PushConstantData
		{
			uSize GeneralData;
			uSize megaBuffer;
			uSize GeometryBuffer;
		};

		const PushConstantData pushConstantData = {
			.GeneralData = 0,
			.megaBuffer = m_Scene->GetDeviceMegaBufferPtr(),
			.GeometryBuffer = m_Scene->GetDeviceGeometryBufferPtr()
		};

		using namespace OWC::Graphics;
		m_RayTracingRenderPass = Renderer::GetStaticRayTracingPass();
		Renderer::PushConstant(m_RayTracingRenderPass, *m_RayTracingShader, sizeof(PushConstantData), &pushConstantData);
		Renderer::PipelineBind(m_RayTracingRenderPass, *m_RayTracingShader);
		Renderer::BindUniform(m_RayTracingRenderPass, *m_RayTracingShader);
		Renderer::BindTexture(m_RayTracingRenderPass, *m_RayTracingShader, 0, 0);
		Renderer::RayTrace(m_RayTracingRenderPass, *m_RayTracingShader, 1);
		Renderer::EndPass(m_RayTracingRenderPass);

		m_DisplayRenderPass = Renderer::BeginPass();
		Renderer::PipelineBind(m_DisplayRenderPass, *m_DisplayShader);
		Renderer::BindUniform(m_DisplayRenderPass, *m_DisplayShader);
		Renderer::BindTexture(m_DisplayRenderPass, *m_DisplayShader, 1, 0);
		Renderer::Draw(m_DisplayRenderPass, 6);
		Renderer::EndPass(m_DisplayRenderPass);
	}

	void GPURayTracerRenderer::SetupPipeline()
	{
		using namespace OWC::Graphics;

		{
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

			std::vector<BindingDescription> closestHitBindingDescriptions = {
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
					.descriptorType = closestHitBindingDescriptions,
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

			m_RayTracingShader = BaseShader::CreateRTShader(shaderDatas);
			m_RayTracingShader->BindTexture(0, m_RenderTarget);
			m_RayTracingShader->BindTLAS(1, m_Scene->GetTLAS());
		}

		{
			std::vector<BindingDescription> fragmentBindingDescriptions = {
				{
					.descriptorCount = 1,
					.binding = 0,
					.descriptorType = DescriptorType::UniformBuffer,
					.stageFlags = ShaderType::Fragment
				},
				{
					.descriptorCount = 1,
					.binding = 1,
					.descriptorType = DescriptorType::CombinedImageSampler,
					.stageFlags = ShaderType::Fragment
				}
			};

			auto shaderSrc = LoadFileToBytecode<u32>("../ShaderSrc/CPURayTracerShaders/Image.spv");

			std::vector<ShaderData> shaderDatas = {
				{
					.bytecode = shaderSrc,
					.type = ShaderType::Vertex,
					.language = ShaderData::ShaderLanguage::SPIRV,
					.descriptorType = {},
					.entryPoint = "vertexMain"
				},
				{
					.bytecode = shaderSrc,
					.type = ShaderType::Fragment,
					.language = ShaderData::ShaderLanguage::SPIRV,
					.descriptorType = fragmentBindingDescriptions,
					.entryPoint = "fragmentMain"
				}
			};

			m_DisplayShader = BaseShader::CreateShader(shaderDatas);
			m_DisplayShader->BindUniform(0, m_UniformBuffer);
			m_DisplayShader->BindTexture(1, m_RenderTarget);
		}
	}
}
