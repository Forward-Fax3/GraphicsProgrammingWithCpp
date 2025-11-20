#include "VulkanContext.hpp"
#include "VulkanCore.hpp"

#include <vulkan/vulkan.hpp>
#include <SDL3/SDL_vulkan.h>

#include <iostream>


namespace OWC::Graphics
{
	VulkanContext::VulkanContext(const SDL_Window& windowHandle) // TODO: validation layers and debug messenger
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

		std::vector<const char*> validationLayers;
		if constexpr (IsDebug)
		{
			if (IsExtentionAvailable(instanceExtertionProperties, VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
				extentions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

			validationLayers.push_back("VK_LAYER_KHRONOS_validation");
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

		auto physcalDevices = vkCore.GetVKInstance().enumeratePhysicalDevices();

		vkCore.SetPhysicalDevice(physcalDevices.front()); // TODO: select proper physical device

		auto queueFamilies = vkCore.GetPhysicalDev().getQueueFamilyProperties();
		constexpr size_t sizeMax = std::numeric_limits<size_t>::max();
		[[maybe_unused]] size_t presentQueueFamilyIndex = sizeMax;
		size_t graphicsQueueFamilyIndex = sizeMax;
		[[maybe_unused]] size_t computeQueueFamilyIndex = sizeMax;
		[[maybe_unused]] size_t transferQueueFamilyIndex = sizeMax;

		for (size_t i = 0; i < queueFamilies.size(); i++) // try to find all queue families in one loop and have different queue families if possible
		{
			// TODO: add surface handle
//			if (presentQueueFamilyIndex == sizeMax)
//			{
//				const vk::Bool32 presentSupport = vkCore.GetPhysicalDev().getSurfaceSupportKHR(static_cast<uint32_t>(i), vk::SurfaceKHR()); // TODO: surface handle
//				if (presentSupport)
//					presentQueueFamilyIndex = i;
//			}

			if (graphicsQueueFamilyIndex == sizeMax && (queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics) == vk::QueueFlagBits::eGraphics)
				graphicsQueueFamilyIndex = i;

			else if (computeQueueFamilyIndex == sizeMax && (queueFamilies[i].queueFlags & vk::QueueFlagBits::eCompute) == vk::QueueFlagBits::eCompute)
				computeQueueFamilyIndex = i;

			// prefer a dedicated transfer queue family
			else if (transferQueueFamilyIndex == sizeMax && (queueFamilies[i].queueFlags & vk::QueueFlagBits::eTransfer) == vk::QueueFlagBits::eTransfer
				&& (queueFamilies[i].queueFlags & vk::QueueFlags(vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute)) == vk::QueueFlags(0))
				transferQueueFamilyIndex = i;

			if ((graphicsQueueFamilyIndex /*| presentQueueFamilyIndex*/ | computeQueueFamilyIndex | transferQueueFamilyIndex) != sizeMax)
				break;
		}

		// check if compute queue family is found, otherwise find any even if it shares with others
		if (computeQueueFamilyIndex == sizeMax)
			for (size_t i = 0; i < queueFamilies.size(); i++)
				if ((queueFamilies[i].queueFlags & vk::QueueFlagBits::eCompute) == vk::QueueFlagBits::eCompute)
				{
					computeQueueFamilyIndex = i;
					break;
				}

		// check if transfer queue family is found, otherwise find any even if it shares with others
		if (transferQueueFamilyIndex == sizeMax)
			for (size_t i = 0; i < queueFamilies.size(); i++)
				if ((queueFamilies[i].queueFlags & vk::QueueFlagBits::eTransfer) == vk::QueueFlagBits::eTransfer)
				{
					transferQueueFamilyIndex = i;
					break;
				}

		// if no transfer queue found, use graphics queue
		if (transferQueueFamilyIndex != sizeMax)
			transferQueueFamilyIndex = graphicsQueueFamilyIndex;

		std::cout << "Selected Queue Families: \n"
			<< " Graphics: " << graphicsQueueFamilyIndex << "\n"
			<< " Compute: " << computeQueueFamilyIndex << "\n"
			<< " Transfer: " << transferQueueFamilyIndex << "\n";
		
		// TODO: check queue family index validity

		float queuePriority = 1.0f;
		vk::DeviceQueueCreateInfo deviceGraphicsQueueCreateInfo;
		deviceGraphicsQueueCreateInfo
			.setQueueFamilyIndex(static_cast<uint32_t>(graphicsQueueFamilyIndex))
			.setQueueCount(1)
			.setPQueuePriorities(&queuePriority);

		vk::DeviceQueueCreateInfo deviceComputeQueueCreateInfo;
		deviceComputeQueueCreateInfo
			.setQueueFamilyIndex(static_cast<uint32_t>(computeQueueFamilyIndex))
			.setQueueCount(1)
			.setPQueuePriorities(&queuePriority);

		vk::DeviceQueueCreateInfo deviceTransferQueueCreateInfo;
		deviceTransferQueueCreateInfo
			.setQueueFamilyIndex(static_cast<uint32_t>(transferQueueFamilyIndex))
			.setQueueCount(1)
			.setPQueuePriorities(&queuePriority);

		std::array<vk::DeviceQueueCreateInfo, 3> deviceQueueCreateInfos
		{
			deviceGraphicsQueueCreateInfo,
			deviceComputeQueueCreateInfo,
			deviceTransferQueueCreateInfo
		};

		vkCore.SetDevice(
			vkCore.GetPhysicalDev().createDevice(
				vk::DeviceCreateInfo()
					.setQueueCreateInfoCount(3)
					.setPQueueCreateInfos(deviceQueueCreateInfos.data())
			)
		);
	}

	VulkanContext::~VulkanContext()
	{
		auto& vkCore = VulkanCore::GetConstInstance();

		vkCore.GetDevice().destroy();
		vkCore.GetVKInstance().destroy();
		VulkanCore::Shutdown();
	}

	void VulkanContext::SwapBuffers()
	{

	}

	void VulkanContext::WaitForIdle()
	{
		VulkanCore::GetConstInstance().GetDevice().waitIdle();
	}
}