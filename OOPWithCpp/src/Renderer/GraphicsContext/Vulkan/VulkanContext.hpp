#pragma once
#include "GraphicsContext.hpp"
#include <vulkan/vulkan.hpp>


namespace OWC::Graphics
{
	class VulkanContext final : public GraphicsContext
	{
	private:
		struct QueueFamilyIndices
		{
			uint32_t PresentFamily = std::numeric_limits<uint32_t>::max();
			uint32_t GraphicsFamily = std::numeric_limits<uint32_t>::max();
			uint32_t ComputeFamily = std::numeric_limits<uint32_t>::max();
			uint32_t TransferFamily = std::numeric_limits<uint32_t>::max();
			
			std::vector<uint32_t> uniqueIndices = {};

			// helper function to check if all families are found
			[[nodiscard]] inline bool FoundAll() const
			{
				return (PresentFamily | GraphicsFamily |
						ComputeFamily | TransferFamily)
						!= std::numeric_limits<uint32_t>::max();
			}
		};

	public:
		explicit VulkanContext(SDL_Window& windowHandle);
		~VulkanContext() override;
		VulkanContext(const VulkanContext&) = delete;
		VulkanContext& operator=(const VulkanContext&) = delete;
		VulkanContext(VulkanContext&&) = delete;
		VulkanContext& operator=(VulkanContext&&) = delete;

		void FinishRender() override;
		void SwapPresentImage() override;
		void WaitForIdle() override;
#ifndef DIST
		void FlushValidationMessages() override;
#endif

	private:
		void StartInstance();
#ifndef DIST
		void EnableVulkanDebugging();
#endif
		void SurfaceInit(SDL_Window& windowHandle);
		void SelectPhysicalDevice();
		std::pair<bool, uint32_t> IsPhysicalDeviceSuitable(const vk::PhysicalDevice& device);
		void FindQueueFamilies();
		void CheckQueueFamilyValidity(const std::vector<vk::QueueFamilyProperties> queueFamilies);
		void GetAndStoreGlobalQueueFamilies();
		void CreateLogicalDevice();
		void CreateSwapchain();
		void CreateFramebuffers(uint32_t width, uint32_t height);
		void CreateRenderPass();
		void CreateCommandPools() const;

		void DestroySwapchain();

		void RecreateSwapchain();

	private:
#ifndef DIST
		vk::DebugUtilsMessengerEXT m_DebugCallback{};
#endif
		QueueFamilyIndices m_QueueFamilyIndices{};
	};
}
