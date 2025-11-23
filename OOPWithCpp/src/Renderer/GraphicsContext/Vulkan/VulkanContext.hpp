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
		};

	public:
		explicit VulkanContext(SDL_Window& windowHandle);
		~VulkanContext() override;

		void SwapBuffers() override;
		void WaitForIdle() override;

	private:
		void StartInstance();
		void EnableVulkanDebugging();
		void SurfaceInit(SDL_Window& windowHandle);
		// void SelectPhysicalDevice(); TODO: implement physical device selection
		QueueFamilyIndices FindQueueFamilies();
		void CheckQueueFamilyValidity(const std::vector<vk::QueueFamilyProperties> queueFamilies, QueueFamilyIndices& indices);
		void CreateLogicalDevice(const QueueFamilyIndices& indices);

	private:
		std::enable_if_t<!IsDistributionMode(), vk::DebugUtilsMessengerEXT> m_DebugCallback{};
	};
}
