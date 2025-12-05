#include "VulkanCore.hpp"

#include <mutex>


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

	std::vector<vk::CommandBuffer> VulkanCore::GetGraphicsCommandBuffer() const
	{
		auto commandBuffers = m_Device.allocateCommandBuffers(
			vk::CommandBufferAllocateInfo()
			.setCommandPool(m_GraphicsCommandPool)
			.setLevel(vk::CommandBufferLevel::ePrimary)
			.setCommandBufferCount(static_cast<uint32_t>(m_SwapchainImageViews.size()))
		);

		for (const auto& cmdBuf : commandBuffers)
			Log<LogLevel::Debug>("Created graphics command buffer: ID: 0x{:x}", std::bit_cast<const uint64_t>(static_cast<const VkCommandBuffer>(cmdBuf)));

		return commandBuffers;
	}

	std::vector<vk::CommandBuffer> VulkanCore::GetComputeCommandBuffer() const
	{
		auto commandBuffers = m_Device.allocateCommandBuffers(
			vk::CommandBufferAllocateInfo()
				.setCommandPool(m_ComputeCommandPool)
				.setLevel(vk::CommandBufferLevel::ePrimary)
				.setCommandBufferCount(static_cast<uint32_t>(m_SwapchainImageViews.size()))
		);
		return commandBuffers;
	}

	std::vector<vk::CommandBuffer> VulkanCore::GetTransferCommandBuffer() const
	{
		auto commandBuffers = m_Device.allocateCommandBuffers(
			vk::CommandBufferAllocateInfo()
				.setCommandPool(m_TransferCommandPool)
				.setLevel(vk::CommandBufferLevel::ePrimary)
				.setCommandBufferCount(static_cast<uint32_t>(m_SwapchainImageViews.size()))
		);
		return commandBuffers;
	}

	std::vector<vk::CommandBuffer> VulkanCore::GetDynamicGraphicsCommandBuffer() const
	{
		auto commandBuffers = m_Device.allocateCommandBuffers(
			vk::CommandBufferAllocateInfo()
			.setCommandPool(m_DynamicGraphicsCommandPool)
			.setLevel(vk::CommandBufferLevel::ePrimary)
			.setCommandBufferCount(static_cast<uint32_t>(m_SwapchainImageViews.size()))
		);

		for (const auto& cmdbuf : commandBuffers)
			Log<LogLevel::Debug>("Allocated dynamic graphics command buffer: ID: 0x{:x}", std::bit_cast<const uint64_t>(static_cast<const VkCommandBuffer>(cmdbuf)));

		return commandBuffers;
	}

	std::vector<vk::CommandBuffer> VulkanCore::GetDynamicComputeCommandBuffer() const
	{
		auto commandBuffers = m_Device.allocateCommandBuffers(
			vk::CommandBufferAllocateInfo()
			.setCommandPool(m_DynamicComputeCommandPool)
			.setLevel(vk::CommandBufferLevel::ePrimary)
			.setCommandBufferCount(static_cast<uint32_t>(m_SwapchainImageViews.size()))
		);
		return commandBuffers;
	}

	std::vector<vk::CommandBuffer> VulkanCore::GetDynamicTransferCommandBuffer() const
	{
		auto commandBuffers = m_Device.allocateCommandBuffers(
			vk::CommandBufferAllocateInfo()
			.setCommandPool(m_DynamicTransferCommandPool)
			.setLevel(vk::CommandBufferLevel::ePrimary)
			.setCommandBufferCount(static_cast<uint32_t>(m_SwapchainImageViews.size()))
		);
		return commandBuffers;
	}

	std::vector<vk::Semaphore> VulkanCore::GetSemaphoresFromNames(std::span<std::string_view> semaphoreNames)
	{
		std::vector<vk::Semaphore> semaphores;
		semaphores.reserve(semaphoreNames.size());

		for (const auto& name : semaphoreNames)
		{
			auto it = m_Semaphores[m_CurrentFrameIndex].find(std::string(name));

			if (it != m_Semaphores[m_CurrentFrameIndex].end())
				semaphores.push_back(it->second);

			else
			{
				for (size_t i = 0; i < m_SwapchainImageViews.size(); ++i)
				{
					vk::SemaphoreCreateInfo semaphoreCreateInfo{};
					semaphoreCreateInfo.setFlags(vk::SemaphoreCreateFlags());
					vk::Semaphore semaphore = m_Device.createSemaphore(semaphoreCreateInfo);
					m_Semaphores[i][std::string(name)] = semaphore;

					Log<LogLevel::Debug>("Created semaphore: name: {}, ID: 0x{:x}", name, std::bit_cast<uint64_t>(static_cast<VkSemaphore>(semaphore)));
				}
				semaphores.push_back(m_Semaphores[m_CurrentFrameIndex][std::string(name)]);
			}
		}

		return semaphores;
	}
}
