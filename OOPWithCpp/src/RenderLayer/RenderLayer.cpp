#include "Core.hpp"
#include "RenderLayer.hpp"
#include "LoadFile.hpp"
#include "Renderer.hpp"

#include "BaseEvent.hpp"
#include "WindowMinimizeEvent.hpp"
#include "WindowRestoreEvent.hpp"
#include "WindowResize.hpp"

#include <array>

#include <glm/gtc/type_ptr.hpp>


namespace OWC
{
	static bool operator>(const Vec2u& lhs, u32 rhs)
	{
		return (lhs.x > rhs) && (lhs.y > rhs);
	}
	
	RenderLayer::RenderLayer(const std::shared_ptr<InterLayerData>& ILD)
		: m_ILD(ILD)
	{
		m_UniformBuffer = Graphics::UniformBuffer::CreateUniformBuffer(sizeof(UniformBufferObject));
		m_Image = Graphics::DynamicTextureBuffer::CreateDynamicTextureBuffer(1, 1);
		std::vector<Vec4> emptyImageData = { Vec4(0.0f) };
		m_Image->UpdateBufferData(emptyImageData);
		SetupPipeline();
		SetupRenderPass();
	}

	void RenderLayer::OnUpdate()
	{
		using namespace OWC::Graphics;

		std::array<std::string_view, 1> waitSemaphorenames = { "ImageReady" };
		std::array<std::string_view, 1> signalSemaphoreNames = { "RenderLayer" };

		UniformBufferObject ubo{
			.divider = 1.0f / static_cast<f32>(m_ILD->numberOfSamples),
			.invGammaValue = m_ILD->invGammaValue
		};

		m_UniformBuffer->UpdateBufferData(std::as_bytes(std::span<const UniformBufferObject>(&ubo, 1)));

		if (m_ILD->ImageUpdates.any())
		{
			if (m_ILD->ImageUpdates[1] && m_ILD->imageScreenSize > 0u) // image resize
			{
				m_Image = DynamicTextureBuffer::CreateDynamicTextureBuffer(m_ILD->imageScreenSize.x, m_ILD->imageScreenSize.y);
				m_Image->UpdateBufferData(m_ILD->imageData);
				SetupPipeline();
				SetupRenderPass();
			}
			else if (m_ILD->imageScreenSize.x == 0 || m_ILD->imageScreenSize.y == 0) // clear image
			{
				m_Image = DynamicTextureBuffer::CreateDynamicTextureBuffer(1, 1);
				std::vector<Vec4> emptyImageData = { Vec4(0.0f) };
				m_Image->UpdateBufferData(emptyImageData);
				SetupPipeline();
				SetupRenderPass();
			}
			else if (m_ILD->ImageUpdates[0]) // update image
			{
				m_Image->UpdateBufferData(m_ILD->imageData);
			}
			m_ILD->ImageUpdates.reset();
		}

		Renderer::RestartRenderPass(m_renderPass);
		Renderer::SubmitRenderPass(m_renderPass, waitSemaphorenames, signalSemaphoreNames);
	}

	void RenderLayer::OnEvent(class BaseEvent& e)
	{
		EventDispatcher dispatcher(e);

		dispatcher.Dispatch<WindowRestore>([this](const WindowRestore& /*event*/) {
			this->SetupRenderPass();
			this->SetActive(true);
			return false;
			});

		if (!IsActive())
			return;

		dispatcher.Dispatch<WindowMinimize>([this](const WindowMinimize& /*event*/) {
			this->SetActive(false);
			return false;
			});

		dispatcher.Dispatch<WindowResize>([this](const WindowResize& /*event*/) {
			this->SetupPipeline();
			this->SetupRenderPass();
			return false;
			});
	}

	void RenderLayer::SetupRenderPass()
	{
		using namespace OWC::Graphics;
		constexpr u32 numberOfVertices = 6;

		m_renderPass = Renderer::BeginPass();
		Renderer::PipelineBind(m_renderPass, *m_Shader);
		Renderer::BindUniform(m_renderPass, *m_Shader);
		Renderer::BindDynamicTexture(m_renderPass, *m_Shader, 1, 0);
		Renderer::Draw(m_renderPass, numberOfVertices);
		Renderer::EndPass(m_renderPass);
	}

	void RenderLayer::SetupPipeline()
	{
		using namespace OWC::Graphics;

		std::vector<BindingDiscription> fragmentBindingDiscriptions = {
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

		std::vector<ShaderData> shaderDatas = {
			{
				.bytecode = LoadFileToBytecode<u32>("../ShaderSrc/Image.vert.spv"),
				.type = ShaderType::Vertex,
				.language = ShaderData::ShaderLanguage::SPIRV,
				.descriptorType = {}
			},
			{
				.bytecode = LoadFileToBytecode<u32>("../ShaderSrc/Image.frag.spv"),
				.type = ShaderType::Fragment,
				.language = ShaderData::ShaderLanguage::SPIRV,
				.descriptorType = fragmentBindingDiscriptions
			}
		};

		m_Shader = BaseShader::CreateShader(shaderDatas);
		m_Shader->BindUniform(0, m_UniformBuffer);
		m_Shader->BindDynamicTexture(1, m_Image);
	}
}
