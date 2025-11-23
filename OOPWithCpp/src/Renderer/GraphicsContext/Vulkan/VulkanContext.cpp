#include <limits>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_to_string.hpp>
#include <SDL3/SDL_vulkan.h>

#include "VulkanContext.hpp"
#include "VulkanCore.hpp"
#include "Log.hpp"


PFN_vkCreateDebugUtilsMessengerEXT pfnVkCreateDebugUtilsMessengerEXT;
PFN_vkDestroyDebugUtilsMessengerEXT pfnVkDestroyDebugUtilsMessengerEXT;

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugUtilsMessengerEXT(
	VkInstance instance,
	const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkDebugUtilsMessengerEXT* pMessenger)
{
	return pfnVkCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDebugUtilsMessengerEXT(
	VkInstance instance,
	VkDebugUtilsMessengerEXT messenger,
	VkAllocationCallbacks const* pAllocator)
{
	return pfnVkDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
}

namespace OWC::Graphics
{
	VulkanContext::VulkanContext(SDL_Window& windowHandle)
	{
		VulkanCore::Init();
		
		try
		{
			StartInstance();
			EnableVulkanDebugging();
			SurfaceInit(windowHandle);
			auto physcalDevices = VulkanCore::GetConstInstance().GetVKInstance().enumeratePhysicalDevices();
			VulkanCore::GetInstance().SetPhysicalDevice(physcalDevices.front()); // TODO: select proper physical device
			auto queueFamilyIndices = FindQueueFamilies();
			CreateLogicalDevice(queueFamilyIndices);
		}
		catch (const vk::SystemError& err)
		{
			Log<LogLevel::Critical>("Vulkan Context initialization failed: {}", err.what());
		}
		catch (const std::exception& ex)
		{
			Log<LogLevel::Critical>("Vulkan Context initialization failed: {}", ex.what());
		}
	}

	VulkanContext::~VulkanContext()
	{
		auto& vkCore = VulkanCore::GetConstInstance();

		SDL_Vulkan_DestroySurface(vkCore.GetVKInstance(), vkCore.GetSurface(), nullptr);
		vkCore.GetDevice().destroy();
		if constexpr (!IsDistributionMode())
			if (m_DebugCallback)
				vkCore.GetVKInstance().destroyDebugUtilsMessengerEXT(m_DebugCallback);
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

	void VulkanContext::StartInstance()
	{
		std::vector<const char*> extentions;
		{
			uint32_t numberOfSDLExtensions = 0;
			const auto extentionsTemp = SDL_Vulkan_GetInstanceExtensions(&numberOfSDLExtensions);

			if constexpr (!IsDistributionMode()) // +2 for debug utils and get physical device properties 2
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
		if constexpr (!IsDistributionMode())
		{
			if (IsExtentionAvailable(instanceExtertionProperties, VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
				extentions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			else
				Log<LogLevel::Critical>("Vulkan extension {} is not available", VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

			validationLayers.push_back("VK_LAYER_KHRONOS_validation");
			createInfo.setEnabledLayerCount(static_cast<uint32_t>(validationLayers.size()));
			createInfo.setPpEnabledLayerNames(validationLayers.data());
		}

		createInfo.setEnabledExtensionCount(static_cast<uint32_t>(extentions.size()));
		createInfo.setPpEnabledExtensionNames(extentions.data());
		VulkanCore::GetInstance().SetInstance(vk::createInstance(createInfo));
	}

	void VulkanContext::EnableVulkanDebugging()
	{
		if constexpr (!IsDistributionMode())
		{
			pfnVkCreateDebugUtilsMessengerEXT = std::bit_cast<PFN_vkCreateDebugUtilsMessengerEXT>(VulkanCore::GetConstInstance().GetVKInstance().getProcAddr("vkCreateDebugUtilsMessengerEXT"));
			pfnVkDestroyDebugUtilsMessengerEXT = std::bit_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(VulkanCore::GetConstInstance().GetVKInstance().getProcAddr("vkDestroyDebugUtilsMessengerEXT"));
			if (!pfnVkCreateDebugUtilsMessengerEXT || !pfnVkDestroyDebugUtilsMessengerEXT)
				Log<LogLevel::Critical>("Failed to load Vulkan debug utils functions");

			m_DebugCallback = VulkanCore::GetConstInstance().GetVKInstance().createDebugUtilsMessengerEXT(
				vk::DebugUtilsMessengerCreateInfoEXT()
				.setMessageSeverity(
					vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
					vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
					vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
					vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
				).setMessageType(
					vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
					vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
					vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
				).setPfnUserCallback( // TODO: do more with pCallbackData
					[](
						vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
						vk::DebugUtilsMessageTypeFlagsEXT messageTypes,
						const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
						void* // unused userData
						) -> vk::Bool32
					{
						if (messageSeverity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eError)
							Log<LogLevel::Error>("Vulkan Validation Layer: {}: {}", vk::to_string(messageTypes), pCallbackData->pMessage);
						else if (messageSeverity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
							Log<LogLevel::Warn>("Vulkan Validation Layer: {}: {}", vk::to_string(messageTypes), pCallbackData->pMessage);
						else if (messageSeverity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo)
							Log<LogLevel::Trace>("Vulkan Validation Layer: {}: {}", vk::to_string(messageTypes), pCallbackData->pMessage);
						else
							Log<LogLevel::Trace>("Vulkan Validation Layer: {}: {}", vk::to_string(messageTypes), pCallbackData->pMessage);
						return VK_FALSE;
					}
				)
			);
		}
	}

	void VulkanContext::SurfaceInit(SDL_Window& windowHandle)
	{
		VkSurfaceKHR surface{};
		SDL_Vulkan_CreateSurface(&windowHandle, VulkanCore::GetInstance().GetVKInstance(), nullptr, &surface);
		VulkanCore::GetInstance().SetSurface(vk::SurfaceKHR(surface));
	}

	VulkanContext::QueueFamilyIndices VulkanContext::FindQueueFamilies()
	{
		auto queueFamilies = VulkanCore::GetConstInstance().GetPhysicalDev().getQueueFamilyProperties();
		constexpr uint32_t indexMax = std::numeric_limits<uint32_t>::max();
		QueueFamilyIndices indices;

		for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilies.size()); i++) // try to find all queue families in one loop and have different queue families if possible
		{
			if (indices.PresentFamily == indexMax &&
				VulkanCore::GetConstInstance().GetPhysicalDev().getSurfaceSupportKHR(i, VulkanCore::GetConstInstance().GetSurface()))
					indices.PresentFamily = i;

			if (indices.GraphicsFamily == indexMax && (queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics) == vk::QueueFlagBits::eGraphics)
				indices.GraphicsFamily = i;

			// prefer a dedicated compute queue family
			else if (indices.ComputeFamily == indexMax && (queueFamilies[i].queueFlags & vk::QueueFlagBits::eCompute) == vk::QueueFlagBits::eCompute)
				indices.ComputeFamily = i;

			// prefer a dedicated transfer queue family
			else if (indices.TransferFamily == indexMax && (queueFamilies[i].queueFlags & vk::QueueFlagBits::eTransfer) == vk::QueueFlagBits::eTransfer
				&& (queueFamilies[i].queueFlags & vk::QueueFlags(vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute)) == vk::QueueFlags(0))
				indices.TransferFamily = i;

			// break early if all found
			if ((
					indices.PresentFamily |
					indices.GraphicsFamily |
					indices.ComputeFamily |
					indices.TransferFamily)
				!= indexMax)
				break;
		}

		CheckQueueFamilyValidity(queueFamilies, indices);

		Log<LogLevel::Debug>("Vulkan Queue Families found:");
		Log<LogLevel::Debug>(" Present Queue Family Index: {}", indices.PresentFamily);
		Log<LogLevel::Debug>(" Graphics Queue Family Index: {}", indices.GraphicsFamily);
		Log<LogLevel::Debug>(" Compute Queue Family Index: {}", indices.ComputeFamily);
		Log<LogLevel::Debug>(" Transfer Queue Family Index: {}", indices.TransferFamily);
		Log<LogLevel::NewLine>();

		return indices;
	}

	void VulkanContext::CheckQueueFamilyValidity(const std::vector<vk::QueueFamilyProperties> queueFamilies, QueueFamilyIndices& indices)
	{
		constexpr uint32_t indexMax = std::numeric_limits<uint32_t>::max();

		if (indices.PresentFamily == indexMax)
			Log<LogLevel::Critical>("Failed to find a valid present queue family index");
		else if (indices.GraphicsFamily == indexMax)
			Log<LogLevel::Critical>("Failed to find a valid graphics queue family index");
		else if (indices.ComputeFamily == indexMax) // check if compute queue family is found, otherwise find any even if it shares with others
		{
			Log<LogLevel::Warn>("Failed to find a unique compute queue family index, any compute queue will be searched for now");

			for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilies.size()); i++)
				if ((queueFamilies[i].queueFlags & vk::QueueFlagBits::eCompute) == vk::QueueFlagBits::eCompute)
				{
					indices.ComputeFamily = i;
					break;
				}
		}

		if (indices.ComputeFamily == indexMax)
			Log<LogLevel::Critical>("Failed to find a valid compute queue family index");
		else if (indices.TransferFamily == indexMax) // check if transfer queue family is found, otherwise find any even if it shares with others
		{
			Log<LogLevel::Trace>("Failed to find a unique transfer queue family index, any transfer queue will be searched for now");

			for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilies.size()); i++)
				if ((queueFamilies[i].queueFlags & vk::QueueFlagBits::eTransfer) == vk::QueueFlagBits::eTransfer)
				{
					indices.TransferFamily = i;
					break;
				}
		}

		// if still not found, use graphics queue family
		if (indices.TransferFamily == indexMax)
		{
			Log<LogLevel::Trace>("Failed to find a transfer queue bit in any family index, graphics queue family will be used implicitly instead");
			indices.TransferFamily = indices.GraphicsFamily;
		}
	}

	void VulkanContext::CreateLogicalDevice(const QueueFamilyIndices& indices)
	{
		size_t numberOfUniqueQueueFamilies = 0;
		float queuePriority = 1.0f;

		vk::DeviceQueueCreateInfo deviceGraphicsQueueCreateInfo;
		vk::DeviceQueueCreateInfo deviceComputeQueueCreateInfo;
		vk::DeviceQueueCreateInfo deviceTransferQueueCreateInfo;

		deviceGraphicsQueueCreateInfo
			.setQueueFamilyIndex(indices.GraphicsFamily)
			.setQueueCount(1)
			.setPQueuePriorities(&queuePriority);

		numberOfUniqueQueueFamilies++;

		if (indices.ComputeFamily != indices.GraphicsFamily)
		{
			numberOfUniqueQueueFamilies++;

			deviceComputeQueueCreateInfo
				.setQueueFamilyIndex(indices.ComputeFamily)
				.setQueueCount(1)
				.setPQueuePriorities(&queuePriority);
		}

		if (indices.TransferFamily != indices.GraphicsFamily && indices.TransferFamily != indices.ComputeFamily)
		{
			numberOfUniqueQueueFamilies++;
			deviceTransferQueueCreateInfo
				.setQueueFamilyIndex(indices.TransferFamily)
				.setQueueCount(1)
				.setPQueuePriorities(&queuePriority);
		}

		
		std::vector<vk::DeviceQueueCreateInfo> deviceQueueCreateInfos;
		deviceQueueCreateInfos.reserve(numberOfUniqueQueueFamilies);
		deviceQueueCreateInfos.emplace_back(deviceGraphicsQueueCreateInfo);

		if (indices.ComputeFamily != indices.GraphicsFamily)
			deviceQueueCreateInfos.emplace_back(deviceComputeQueueCreateInfo);
		if (indices.TransferFamily != indices.GraphicsFamily && indices.TransferFamily != indices.ComputeFamily)
			deviceQueueCreateInfos.emplace_back(deviceTransferQueueCreateInfo);

		VulkanCore::GetInstance().SetDevice(
			VulkanCore::GetConstInstance().GetPhysicalDev().createDevice(
				vk::DeviceCreateInfo()
				.setQueueCreateInfoCount(static_cast<uint32_t>(deviceQueueCreateInfos.size()))
				.setPQueueCreateInfos(deviceQueueCreateInfos.data())
			)
		);
	}
}