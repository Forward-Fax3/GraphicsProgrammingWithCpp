#include "Application.hpp"
#include "VulkanCore.hpp"


namespace OWC::Graphics
{
	std::unique_ptr<VulkanCore> VulkanCore::s_Instance = nullptr;

	vk::CommandBuffer VulkanCore::GetGraphicsCommandBuffer()
	{
		auto commandBuffers = m_Device.allocateCommandBuffers(
			vk::CommandBufferAllocateInfo()
			.setCommandPool(m_GraphicsCommandPool)
			.setLevel(vk::CommandBufferLevel::ePrimary)
			.setCommandBufferCount(1)
		);
		return commandBuffers[0];
	}

	vk::CommandBuffer VulkanCore::GetComputeCommandBuffer()
	{
		auto commandBuffers = m_Device.allocateCommandBuffers(
			vk::CommandBufferAllocateInfo()
			.setCommandPool(m_ComputeCommandPool)
			.setLevel(vk::CommandBufferLevel::ePrimary)
			.setCommandBufferCount(1)
		);
		return commandBuffers[0];
	}

	vk::CommandBuffer VulkanCore::GetTransferCommandBuffer()
	{
		auto commandBuffers = m_Device.allocateCommandBuffers(
			vk::CommandBufferAllocateInfo()
			.setCommandPool(m_TransferCommandPool)
			.setLevel(vk::CommandBufferLevel::ePrimary)
			.setCommandBufferCount(1)
		);
		return commandBuffers[0];
	}

	void VulkanCore::BeginRenderPass(const vk::CommandBuffer& commandBuf, vk::Pipeline pipeline) const
	{
		vk::ClearValue clearColour(vk::ClearColorValue(std::array<float, 4>{0.0, 0.0, 0.0, 1.0}));

		vk::Rect2D rect(
			vk::Offset2D(0, 0),
			{
				static_cast<uint32_t>(Application::GetConstInstance().GetWindowWidth()),
				static_cast<uint32_t>(Application::GetConstInstance().GetWindowHeight())
			});

		vk::RenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.setRenderPass(m_RenderPass)
			.setFramebuffer(m_SwapchainFramebuffers[m_CurrentFrameIndex])
			.setRenderArea(rect)
			.setClearValueCount(1)
			.setPClearValues(&clearColour);

		commandBuf.begin(vk::CommandBufferBeginInfo());
		commandBuf.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
		commandBuf.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
		commandBuf.setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(Application::GetConstInstance().GetWindowWidth()), static_cast<float>(Application::GetConstInstance().GetWindowHeight()), 0.0f, 1.0f));
		commandBuf.setScissor(0, rect);
	}

	void VulkanCore::EndRenderPass(const vk::CommandBuffer& commandBuf) const
	{
		commandBuf.endRenderPass();
		commandBuf.end();
	}

	void VulkanCore::SubmitGraphicsCommandBuffer(const vk::CommandBuffer& commandBuf) const
	{
		if (m_InFlightFence)
			m_Device.destroyFence(m_InFlightFence);

		m_InFlightFence = m_Device.createFence(vk::FenceCreateInfo());

		vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
		vk::SubmitInfo submitInfo(m_ImageAvailableSemaphore, waitDestinationStageMask, commandBuf);

		m_GraphicsQueue.submit(submitInfo, m_InFlightFence);
	}
}
