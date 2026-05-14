//
// Created by forwardfax3 on 15/03/2026.
//

#include "Core.hpp"
#include "OWCRand.hpp"
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
#include "glm/gtx/euler_angles.hpp"


namespace OWC
{
	//static bool operator>(const Vec2u& lhs, u32 rhs)
	//{
	//	return (lhs.x > rhs) && (lhs.y > rhs);
	//}

	GPURayTracerRenderer::GPURayTracerRenderer()
	{
		m_ProjectionMatrix = glm::perspective(glm::radians(45.0f), 16.0f / 9.0f, 0.1f, 100.0f);
		auto& app = Application::GetConstInstance();
		m_Scene = std::make_shared<SponzaPalace>();
		m_RenderTarget = Graphics::TextureBuffer::CreateTextureBuffer(app.GetPixelWidth(), app.GetPixelHeight());
		m_UniformBuffer = Graphics::UniformBuffer::CreateUniformBuffer(sizeof(UniformBufferObject));
		m_GeneralGPUDataBuffer = Graphics::GeneralBuffer::CreateGeneralBuffer(sizeof(GeneralGPUData));
		m_ScreenNeedsRefreshing = true;

		std::vector<Vec4> nullImageData{};
		nullImageData.resize(app.GetPixelWidth() * app.GetPixelHeight(), Vec4(0.0f));
		m_RenderTarget->UpdateBufferData(nullImageData);

		SetupPipeline();
		SetupRenderPass();
		CalculateCamera();
	}

	void GPURayTracerRenderer::OnUpdate()
	{
		using namespace OWC::Graphics;

		if (m_ScreenNeedsRefreshing)
		{
			const auto& app = Application::GetConstInstance();
			m_ScreenNeedsRefreshing = false;
			m_NumberOfSamples = 0;

			std::vector<Vec4> nullImageData{};
			nullImageData.resize(app.GetPixelWidth() * app.GetPixelHeight(), Vec4(0.0f));
			m_RenderTarget->UpdateBufferData(nullImageData);
		}

		m_NumberOfSamples++;
		std::array<std::string_view, 1> rayTracerWaitSemaphoreNames = { "ImageReady" };
		std::array<std::string_view, 1> rayTracerSignalSemaphoreNames = { "RayTracerDone" };
		std::array<std::string_view, 1> displayWaitSemaphoreNames = { "RayTracerDone" };

		UniformBufferObject ubo{
			.divider = 1.0f / static_cast<f32>(m_NumberOfSamples),
			.invGammaValue = 1.0f / 2.2f
		};

		m_UniformBuffer->UpdateBufferData(std::as_bytes(std::span<const UniformBufferObject>(&ubo, 1)));

		auto newRand = Rand::LinearFastRandValue(0u, std::numeric_limits<u32>::max());
		m_GeneralGPUDataBuffer->UpdateBufferData(std::bit_cast<u8*>(&newRand), sizeof(GeneralGPUData::randSeed), offsetof(GeneralGPUData, randSeed));

		Renderer::SubmitRenderPass(m_RayTracingRenderPass, rayTracerWaitSemaphoreNames, rayTracerSignalSemaphoreNames);
		Renderer::SubmitRenderPass(m_DisplayRenderPass, displayWaitSemaphoreNames);
	}

	void GPURayTracerRenderer::ImGuiRender()
	{
		bool cameraMoved = false;
		ImGui::Begin("GPU Renderer");
		ImGui::Text("Number of samples: %zu", m_NumberOfSamples);
		ImGui::Text("Ray tracer");
		cameraMoved |= ImGui::DragFloat3("Camera Position", glm::value_ptr(m_CameraPosition), 0.1f);
		cameraMoved |= ImGui::DragFloat3("Camera Rotation", glm::value_ptr(m_CameraRotation), 0.1f);
		ImGui::End();

		if (cameraMoved)
			CalculateCamera();
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
			.GeneralData = m_GeneralGPUDataBuffer->GetDeviceBufferPtr(),
			.megaBuffer = m_Scene->GetDeviceMegaBufferPtr(),
			.GeometryBuffer = m_Scene->GetDeviceGeometryBufferPtr()
		};

		Log<LogLevel::Debug>("GeneralData: {}, megaBuffer: {}, GeometryBuffer: {}", pushConstantData.GeneralData, pushConstantData.megaBuffer, pushConstantData.GeometryBuffer);

		using namespace OWC::Graphics;
		m_RayTracingRenderPass = Renderer::GetStaticRayTracingPass();
		Renderer::TransitionImage(m_RayTracingRenderPass, m_RenderTarget, AccessMask::ShaderWrite, StageMask::RayTracing, ImageLayout::General);
		Renderer::PipelineBind(m_RayTracingRenderPass, *m_RayTracingShader);
		Renderer::PushConstant(m_RayTracingRenderPass, *m_RayTracingShader, sizeof(PushConstantData), &pushConstantData);
		Renderer::BindUniform(m_RayTracingRenderPass, *m_RayTracingShader);
		Renderer::BindTexture(m_RayTracingRenderPass, *m_RayTracingShader, 0, 0);
		Renderer::RayTrace(m_RayTracingRenderPass, *m_RayTracingShader, 1);
		Renderer::EndPass(m_RayTracingRenderPass);

		m_DisplayRenderPass = Renderer::GetStaticRenderPass();
		Renderer::TransitionImage(m_DisplayRenderPass, m_RenderTarget, AccessMask::ShaderRead, StageMask::FragmentShader, ImageLayout::General);
		Renderer::BeginRasterPass(m_DisplayRenderPass);
		Renderer::PipelineBind(m_DisplayRenderPass, *m_DisplayShader);
		Renderer::BindUniform(m_DisplayRenderPass, *m_DisplayShader);
		Renderer::BindTexture(m_DisplayRenderPass, *m_DisplayShader, 1, 0);
		Renderer::Draw(m_DisplayRenderPass, 6);
		Renderer::EndRasterPass(m_DisplayRenderPass);
		Renderer::TransitionImage(m_DisplayRenderPass, m_RenderTarget, AccessMask::None, StageMask::TopOfPipe, ImageLayout::General);
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

			auto shaderSrc = LoadFileToBytecode<u32>("../ShaderSrc/GPURayTracerShaders/RayTracing.spv");

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
					.type = ShaderType::RayMiss,
					.language = ShaderData::ShaderLanguage::SPIRV,
					.descriptorType = {},
					.entryPoint = "missShader",
				},
				{
					.bytecode = shaderSrc,
					.type = ShaderType::RayClosestHit,
					.language = ShaderData::ShaderLanguage::SPIRV,
					.descriptorType = closestHitBindingDescriptions,
					.entryPoint = "closestHitShader",
				},
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
					.descriptorType = DescriptorType::StorageImage,
					.stageFlags = ShaderType::Fragment
				}
			};

			auto shaderSrc = LoadFileToBytecode<u32>("../ShaderSrc/GPURayTracerShaders/DisplayImage.spv");

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

	void GPURayTracerRenderer::CalculateCamera()
	{
		const Vec3 rotation = glm::radians(m_CameraRotation) + Vec3(glm::pi<f32>(), 0.0f, 0.0f);
		const Mat4 rotMat = glm::eulerAngleYXZ(rotation.y, rotation.x, rotation.z);
		const Mat4 pos = glm::translate(Mat4(1.0f), m_CameraPosition);
		const Mat4 invView = pos * rotMat;

		GeneralGPUData generalGPUData {
			.InvProjection = glm::inverse(m_ProjectionMatrix),
			.InvViewMatrix = glm::transpose(invView),
			.randSeed = Rand::LinearFastRandValue(0u, std::numeric_limits<u32>::max())
		};

		m_GeneralGPUDataBuffer->UpdateBufferData(std::bit_cast<u8*>(&generalGPUData));
		m_ScreenNeedsRefreshing = true;
	}
}
