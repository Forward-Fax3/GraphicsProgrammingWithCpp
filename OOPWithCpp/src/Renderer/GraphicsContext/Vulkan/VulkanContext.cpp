#include "VulkanContext.hpp"
#include "VulkanCore.hpp"

#include <vulkan/vulkan.hpp>
#include <SDL3/SDL_vulkan.h>


namespace OWC::Graphics
{


	VulkanContext::VulkanContext(const SDL_Window& windowHandle)
	{
		const bool IsDebug = IsDebugMode();

		(void)windowHandle; // unused for now
		const auto runtimeVersion = vk::enumerateInstanceVersion();
		if (runtimeVersion < g_VulkanVersion)
		{
			// TODO: error message
			exit(4);
		}

		VulkanCore::Init();
		auto& vkCore = VulkanCore::GetInstance();

		std::vector<const char*> extentions;
		{
			uint32_t numberOfSDLExtensions = 0;
			const auto extentionsTemp = SDL_Vulkan_GetInstanceExtensions(&numberOfSDLExtensions);

			if constexpr (IsDebug) // +2 for debug utils and get physical device properties 2
				extentions.reserve(numberOfSDLExtensions + 2);
			else // +1 for get physical device properties 2
				extentions.reserve(numberOfSDLExtensions + 1);

			extentions.reserve(numberOfSDLExtensions);
			for (size_t i = 0; i < numberOfSDLExtensions; i++)
				extentions.emplace_back(extentionsTemp[i]);
		}

		vk::InstanceCreateInfo createInfo;

		auto instanceExtertionProperties = vk::enumerateInstanceExtensionProperties();

		if (IsExtentionAvailable(instanceExtertionProperties, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME))
			extentions.emplace_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

		if constexpr (IsDebug)
		{
			if (IsExtentionAvailable(instanceExtertionProperties, VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
				extentions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

			const std::vector<const char*> validationLayers = {
				"VK_LAYER_KHRONOS_validation"
			};
			createInfo.setEnabledLayerCount(static_cast<uint32_t>(validationLayers.size()));
			createInfo.setPpEnabledLayerNames(validationLayers.data());
		}

		createInfo.setEnabledExtensionCount(static_cast<uint32_t>(extentions.size()));
		createInfo.setPpEnabledExtensionNames(extentions.data());
		vkCore.SetInstance(vk::createInstance(createInfo));

		if constexpr (IsDebug)
		{
			// TODO: setup debug messenger
		}


	}

	VulkanContext::~VulkanContext()
	{
		vkDestroyInstance(VulkanCore::GetConstInstance().GetVKInstance(), nullptr); // TODO: allocation callbacks
		VulkanCore::Shutdown();
	}

	void VulkanContext::SwapBuffers()
	{

	}

	void VulkanContext::WaitForIdle()
	{
		vkQueueWaitIdle(VulkanCore::GetConstInstance().GetGraphicsQueue());
	}
}