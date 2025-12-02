#include <mutex>

#include "VulkanCore.hpp"


namespace OWC::Graphics
{
	std::unique_ptr<VulkanCore> VulkanCore::s_Instance = nullptr;

	void VulkanCore::AddRenderPassData(const std::shared_ptr<RenderPassData>& data)
	{
		auto vulkanRenderPass = std::dynamic_pointer_cast<VulkanRenderPass>(data);
		if (!vulkanRenderPass)
		{
			Log<LogLevel::Error>("Failed to add RenderPassData: Invalid VulkanRenderPass pointer.");
			return;
		}

		std::scoped_lock lock(m_RenderPassesMutex);
		m_RenderPassDatas.push_back(vulkanRenderPass);
	}

	void VulkanCore::ResetRenderPassDatas()
	{
		std::scoped_lock lock(m_RenderPassesMutex);
		m_RenderPassDatas.clear();
	}

	vk::CommandBuffer VulkanCore::GetGraphicsCommandBuffer() const
	{
		auto commandBuffers = m_Device.allocateCommandBuffers(
			vk::CommandBufferAllocateInfo()
				.setCommandPool(m_GraphicsCommandPool)
				.setLevel(vk::CommandBufferLevel::ePrimary)
				.setCommandBufferCount(1)
		);
		return commandBuffers[0];
	}

	vk::CommandBuffer VulkanCore::GetComputeCommandBuffer() const
	{
		auto commandBuffers = m_Device.allocateCommandBuffers(
			vk::CommandBufferAllocateInfo()
				.setCommandPool(m_ComputeCommandPool)
				.setLevel(vk::CommandBufferLevel::ePrimary)
				.setCommandBufferCount(1)
		);
		return commandBuffers[0];
	}

	vk::CommandBuffer VulkanCore::GetTransferCommandBuffer() const
	{
		auto commandBuffers = m_Device.allocateCommandBuffers(
			vk::CommandBufferAllocateInfo()
				.setCommandPool(m_TransferCommandPool)
				.setLevel(vk::CommandBufferLevel::ePrimary)
				.setCommandBufferCount(1)
		);
		return commandBuffers[0];
	}
}
