#include "VulkanRenderPass.hpp"
#include "VulkanCore.hpp"
#include "VulkanShader.hpp"
#include "Log.hpp"

#include "Application.hpp"


namespace OWC::Graphics
{
	VulkanRenderPass::VulkanRenderPass(RenderPassType type)
		: RenderPassData(type)
	{
		const VulkanCore& vkCore = VulkanCore::GetConstInstance();

		vk::ClearValue clearColour(vk::ClearColorValue(std::array<float, 4>{0.0, 0.0, 0.0, 1.0}));

		vk::Rect2D rect(
			vk::Offset2D(0, 0),
			{
				static_cast<uint32_t>(Application::GetConstInstance().GetWindowWidth()),
				static_cast<uint32_t>(Application::GetConstInstance().GetWindowHeight())
			});

		m_CommandBuffers.reserve(vkCore.GetSwapchainImages().size());

		for (size_t i = 0; i < vkCore.GetSwapchainImages().size(); ++i)
		{
			vk::CommandBuffer cmdbuf = vkCore.GetGraphicsCommandBuffer();

			cmdbuf.begin(vk::CommandBufferBeginInfo());

			vk::RenderPassBeginInfo renderPassInfo = {};
			renderPassInfo.setRenderPass(vkCore.GetRenderPass())
				.setFramebuffer(vkCore.GetSwapchainFramebuffers()[i])
				.setRenderArea(rect)
				.setClearValueCount(1)
				.setPClearValues(&clearColour);

			cmdbuf.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
			cmdbuf.setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(Application::GetConstInstance().GetWindowWidth()), static_cast<float>(Application::GetConstInstance().GetWindowHeight()), 0.0f, 1.0f));
			cmdbuf.setScissor(0, rect);

			m_CommandBuffers.push_back(cmdbuf);
		}
	}

	VulkanRenderPass::~VulkanRenderPass()
	{
		const VulkanCore& vkCore = VulkanCore::GetConstInstance();
		const vk::Device& device = vkCore.GetDevice();

		for (auto& cmdBuf : m_CommandBuffers)
			device.freeCommandBuffers(vkCore.GetGraphicsCommandPool(), cmdBuf);

		device.destroyFence(m_Fence);
	}

	void VulkanRenderPass::AddPipeline(const BaseShader& shader)
	{
		const auto& vulkanShader = dynamic_cast<const VulkanShader&>(shader);
		for (auto& cmdBuf : m_CommandBuffers)
			cmdBuf.bindPipeline(GetPipelineBindPoint(), vulkanShader.GetPipeline());
	}

	void VulkanRenderPass::Draw(uint32_t vertexCount, uint32_t instanceCount /*= 1*/, uint32_t firstVertex /*= 0*/, uint32_t firstInstance /*= 0*/)
	{
		for (auto& cmdBuf : m_CommandBuffers)
			cmdBuf.draw(vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void VulkanRenderPass::EndRenderPass()
	{
		for (auto& cmdBuf : m_CommandBuffers)
		{
			cmdBuf.endRenderPass();
			cmdBuf.end();
		}

		m_Fence = VulkanCore::GetConstInstance().GetDevice().createFence(vk::FenceCreateInfo());
	}

	vk::PipelineBindPoint VulkanRenderPass::GetPipelineBindPoint() const
	{
		switch (GetRenderPassType())
		{
		using enum RenderPassType;
		using enum vk::PipelineBindPoint;
		case Graphics:
			return eGraphics;
		case Compute:
			return eCompute;
		case Transfer:
			// Transfer operations typically use the graphics pipeline in Vulkan
			return eCompute;
		default:
			Log<LogLevel::Critical>("VulkanRenderPass::GetPipelineBindPoint: Unknown RenderPassType");
		}
	}

	void VulkanRenderPass::submitRenderPass()
	{
		VulkanCore& vkCore = VulkanCore::GetInstance();

		vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
		vk::SubmitInfo submitInfo(vkCore.GetImageAvailableSemaphore(), waitDestinationStageMask, m_CommandBuffers[vkCore.GetCurrentFrameIndex()]);

		vkCore.GetGraphicsQueue().submit(submitInfo, m_Fence);
	}

	void VulkanRenderPass::RestartRenderPass()
	{
		VulkanCore::GetConstInstance().GetDevice().resetFences(m_Fence);
	}
}