#include "Application.hpp"
#include "Renderer.hpp"

#include "VulkanRenderPass.hpp"


namespace OWC::Graphics
{
	std::shared_ptr<GraphicsContext> Renderer::s_GC = nullptr;

	RendererAPI Renderer::s_API = RendererAPI::Vulkan;

	void Renderer::Init()
	{
		s_GC = Application::GetInstance().GetWindow().GetGraphicsContextPtr().lock();
	}

	void Renderer::Shutdown()
	{
		s_GC.reset();
	}

	void Renderer::FinishRender()
	{
		s_GC->FinishRender();
	}

	std::shared_ptr<OWC::Graphics::RenderPassData> Renderer::GetDynamicPass()
	{
		switch (s_API) // Switch statement for future APIs
		{
		case OWC::Graphics::RendererAPI::Vulkan:
			return std::make_shared<VulkanRenderPass>(RenderPassType::Dynamic);
		default:
			return nullptr;
		}
	}

	std::shared_ptr<RenderPassData> Renderer::BeginPass()
	{
		switch (s_API) // Switch statement for future APIs
		{
		case OWC::Graphics::RendererAPI::Vulkan:
			return std::make_shared<VulkanRenderPass>(RenderPassType::Static);
		default:
			return nullptr;
		}
	}

	void Renderer::BeginDynamicPass(const std::shared_ptr<RenderPassData>& data)
	{
		data->BeginDynamicPass();
	}

	void Renderer::Draw(const std::shared_ptr<RenderPassData>& data, uint32_t vertexCount, uint32_t instanceCount /*= 1*/, uint32_t firstVertex /*= 0*/, uint32_t firstInstance /*= 0*/)
	{
		data->Draw(vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void Renderer::PipelineBind(const std::shared_ptr<RenderPassData>& data, const BaseShader& shader)
	{
		data->AddPipeline(shader);
	}

	void Renderer::EndPass(const std::shared_ptr<RenderPassData>& data)
	{
		data->EndRenderPass();
	}

	void Renderer::SubmitRenderPass(const std::shared_ptr<RenderPassData>& data, std::span<std::string_view> waitSemaphoreNames, std::span<std::string_view> startSemaphoreNames)
	{
		data->submitRenderPass(waitSemaphoreNames, startSemaphoreNames);
		s_GC->AddRenderPassData(data);
	}

	void Renderer::RestartRenderPass(const std::shared_ptr<RenderPassData>& data)
	{
		data->RestartRenderPass();
	}

	void Renderer::DrawImGui(const std::shared_ptr<RenderPassData>& data, ImDrawData* drawData)
	{
		data->DrawImGui(drawData);
	}
};