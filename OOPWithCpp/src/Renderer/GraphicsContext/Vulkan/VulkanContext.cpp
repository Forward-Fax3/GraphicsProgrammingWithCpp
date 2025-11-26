#include <limits>
#include <mutex>

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

static void PrintVulkanDebugMessages(vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity, const std::vector<std::string>& s_MessagesLogged)
{
	using enum vk::DebugUtilsMessageSeverityFlagBitsEXT;
	using enum OWC::LogLevel;

	std::string loggedMessage;
	for (const auto& msg : s_MessagesLogged)
		loggedMessage += "\t " + msg + "\n";

	if (messageSeverity & eError)
		OWC::Log<Error>("Vulkan Validation Layer:\n{}", loggedMessage);
	else if (messageSeverity & eWarning)
		OWC::Log<Warn>("Vulkan Validation Layer:\n{}", loggedMessage);
	else if (messageSeverity & eInfo)
		OWC::Log<Trace>("Vulkan Validation Layer:\n{}", loggedMessage);
	else
		OWC::Log<Trace>("Vulkan Validation Layer:\n{}", loggedMessage);
}

static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugMessageFunc( // TODO: add objects info logging and add auto flush after certain amount of messages or time
	vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	vk::DebugUtilsMessageTypeFlagsEXT messageTypes,
	vk::DebugUtilsMessengerCallbackDataEXT const* pCallbackData,
	void* /*pUserData*/)
{
	static std::mutex s_Mutex;
	static std::vector<std::string> s_MessagesLogged;
	static int32_t lastMessageID = std::numeric_limits<int32_t>::max();
	static vk::DebugUtilsMessageSeverityFlagBitsEXT lastMessageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT();
	static bool firstMessage = true;
	(void)s_Mutex;

	if (pCallbackData == nullptr)
	{
		if (s_MessagesLogged.empty())
			return VK_FALSE;

		std::lock_guard lock(s_Mutex);
		PrintVulkanDebugMessages(lastMessageSeverity, s_MessagesLogged);
		s_MessagesLogged.clear();
		lastMessageID = std::numeric_limits<int32_t>::max();
		lastMessageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT();
		firstMessage = true;
		return VK_FALSE;
	}

	std::string str = std::format(
		"{} {}: {}",
		pCallbackData->pMessageIdName,
		vk::to_string(messageTypes),
		pCallbackData->pMessage ? pCallbackData->pMessage : "NULL"
	);

	if (pCallbackData->queueLabelCount > 0)
	{
		str += "\n\t Queue Labels:";
		for (uint32_t i = 0; i < pCallbackData->queueLabelCount; i++)
		{
			str += std::format("\n\t\t labelName = <{}>",
				pCallbackData->pQueueLabels[i].pLabelName ? pCallbackData->pQueueLabels[i].pLabelName : "NULL");
		}
	}
	if (pCallbackData->cmdBufLabelCount > 0)
	{
		str += "\n\t CommandBuffer Labels:";
		for (uint32_t i = 0; i < pCallbackData->cmdBufLabelCount; i++)
		{
			str += std::format("\n\t\t labelName = <{}>",
				pCallbackData->pCmdBufLabels[i].pLabelName ? pCallbackData->pCmdBufLabels[i].pLabelName : "NULL");
		}
	}
//	if (pCallbackData->objectCount > 0 /* && !std::string("NULL").compare(pCallbackData->pObjects[0].pObjectName) */)
//	{
//		str += "\n\t Objects:";
//		for (uint32_t i = 0; i < pCallbackData->objectCount; i++)
//		{
//			str += std::format(
//				"\n\t\t objectName = <{}>"
//				"\n\t\t objectType = {}",
//				pCallbackData->pObjects[i].pObjectName ? pCallbackData->pObjects[i].pObjectName : "NULL",
//				vk::to_string(pCallbackData->pObjects[i].objectType)
//			);
//		}
//	}

	// leave the lock as late as possible
	std::lock_guard lock(s_Mutex);
	if (pCallbackData->messageIdNumber == lastMessageID && messageSeverity == lastMessageSeverity)
		s_MessagesLogged.emplace_back(std::move(str));
	else if (firstMessage)
	{
		s_MessagesLogged.emplace_back(std::move(str));
		lastMessageID = pCallbackData->messageIdNumber;
		lastMessageSeverity = messageSeverity;
		firstMessage = false;
	}
	else
	{
		PrintVulkanDebugMessages(lastMessageSeverity, s_MessagesLogged);
		s_MessagesLogged.clear();
		s_MessagesLogged.emplace_back(std::move(str));
		if (messageSeverity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eError)
		{
			// flush immediately on error messages
			PrintVulkanDebugMessages(messageSeverity, s_MessagesLogged);
			s_MessagesLogged.clear();
			lastMessageID = std::numeric_limits<int32_t>::max();
			lastMessageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT();
			firstMessage = true;
		}
		else
		{
			lastMessageID = pCallbackData->messageIdNumber;
			lastMessageSeverity = messageSeverity;
		}
	}

	return VK_FALSE;
}

namespace OWC::Graphics
{
	VulkanContext::VulkanContext(SDL_Window& windowHandle)
	{
		VulkanCore::Init();
		
		try
		{
			StartInstance();
#ifndef DIST
			EnableVulkanDebugging();
#endif
			SurfaceInit(windowHandle);
			auto physcalDevices = VulkanCore::GetConstInstance().GetVKInstance().enumeratePhysicalDevices();
			VulkanCore::GetInstance().SetPhysicalDevice(physcalDevices.front()); // TODO: select proper physical device
			auto queueFamilyIndices = FindQueueFamilies();
			CreateLogicalDevice(queueFamilyIndices);
			SetupSwapchain(windowHandle, queueFamilyIndices);
		}
		catch (const vk::SystemError& err)
		{
			Log<LogLevel::Critical>("Vulkan Context initialization failed: {}", err.what());
		}
		catch (const std::exception& ex)
		{
			Log<LogLevel::Critical>("Vulkan Context initialization failed: {}", ex.what());
		}

		// flush any logged messages during initialization
		debugMessageFunc(
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo,
			vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral,
			nullptr,
			nullptr
		);
	}

	VulkanContext::~VulkanContext()
	{
		WaitForIdle();
		auto& vkCore = VulkanCore::GetConstInstance();

		for (const auto& imageView : vkCore.GetSwapchainImageViews())
			vkCore.GetDevice().destroyImageView(imageView);

		vkCore.GetDevice().destroySwapchainKHR(vkCore.GetSwapchain());
		SDL_Vulkan_DestroySurface(vkCore.GetVKInstance(), vkCore.GetSurface(), nullptr);
		vkCore.GetDevice().destroy();
#ifndef DIST
		// flush any remaining logged messages before destroying the debug messenger
		debugMessageFunc(
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo,
			vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral,
			nullptr,
			nullptr
		);

		if (m_DebugCallback)
				vkCore.GetVKInstance().destroyDebugUtilsMessengerEXT(m_DebugCallback);
#endif
		vkCore.GetVKInstance().destroy();

		VulkanCore::Shutdown();
	}

	void VulkanContext::SwapBuffers() // TODO: rename and implement
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

			if constexpr (!IsDistributionMode()) // +3 for debug utils and get physical device properties 2 and surface extensions
				extentions.reserve(numberOfSDLExtensions + 3);
			else // +2 for get physical device properties 2 and surface extensions
				extentions.reserve(numberOfSDLExtensions + 2);

			for (size_t i = 0; i < numberOfSDLExtensions; i++)
				extentions.emplace_back(extentionsTemp[i]);
		}

		vk::InstanceCreateInfo createInfo;

		auto instanceExtertionProperties = vk::enumerateInstanceExtensionProperties();

		for (const auto& ext : extentions)
			if (!IsExtentionAvailable(instanceExtertionProperties, ext))
				Log<LogLevel::Critical>("Vulkan extension {} is not available", ext);

		if (std::ranges::find(extentions, vk::KHRSurfaceExtensionName) == extentions.end())
		{
			if (IsExtentionAvailable(instanceExtertionProperties, vk::KHRSurfaceExtensionName))
				extentions.emplace_back(vk::KHRSurfaceExtensionName);
			else
				Log<LogLevel::Critical>("Vulkan extension {} is not available", vk::KHRSurfaceExtensionName);
		}

		if (IsExtentionAvailable(instanceExtertionProperties, vk::KHRGetPhysicalDeviceProperties2ExtensionName))
			extentions.emplace_back(vk::KHRGetPhysicalDeviceProperties2ExtensionName);
		else
			Log<LogLevel::Critical>("Vulkan extension {} is not available", vk::KHRGetPhysicalDeviceProperties2ExtensionName);

		if (IsExtentionAvailable(instanceExtertionProperties, vk::KHRGetSurfaceCapabilities2ExtensionName))
			extentions.emplace_back(vk::KHRGetSurfaceCapabilities2ExtensionName);
		else
			Log<LogLevel::Critical>("Vulkan extension {} is not available", vk::KHRGetSurfaceCapabilities2ExtensionName);
		
		std::vector<const char*> validationLayers;
		if constexpr (!IsDistributionMode())
		{
			if (IsExtentionAvailable(instanceExtertionProperties, vk::EXTDebugUtilsExtensionName))
			{
				extentions.emplace_back(vk::EXTDebugUtilsExtensionName);

				validationLayers.push_back("VK_LAYER_KHRONOS_validation");
				createInfo.setEnabledLayerCount(static_cast<uint32_t>(validationLayers.size()));
				createInfo.setPpEnabledLayerNames(validationLayers.data());
			}
			else
				Log<LogLevel::Warn>("Vulkan extension {} is not available", vk::EXTDebugUtilsExtensionName);
		}

		createInfo.setEnabledExtensionCount(static_cast<uint32_t>(extentions.size()));
		createInfo.setPpEnabledExtensionNames(extentions.data());
		VulkanCore::GetInstance().SetInstance(vk::createInstance(createInfo));
	}

#ifndef DIST
	void VulkanContext::EnableVulkanDebugging()
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
			).setPfnUserCallback(debugMessageFunc)
		);
	}
#endif

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
			if (indices.FoundAll())
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

	void VulkanContext::CreateLogicalDevice(QueueFamilyIndices& indices)
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

		indices.uniqueIndices.reserve(numberOfUniqueQueueFamilies);
		indices.uniqueIndices.emplace_back(indices.GraphicsFamily);

		if (indices.ComputeFamily != indices.GraphicsFamily)
		{
			deviceQueueCreateInfos.emplace_back(deviceComputeQueueCreateInfo);
			indices.uniqueIndices.emplace_back(indices.ComputeFamily);
		}
		if (indices.TransferFamily != indices.GraphicsFamily && indices.TransferFamily != indices.ComputeFamily)
		{
			deviceQueueCreateInfos.emplace_back(deviceTransferQueueCreateInfo);
			indices.uniqueIndices.emplace_back(indices.TransferFamily);
		}

		const std::vector<const char*> deviceExtensions = {
			vk::KHRSwapchainExtensionName,
			vk::KHRMaintenance1ExtensionName
		};

		VulkanCore::GetInstance().SetDevice(
			VulkanCore::GetConstInstance().GetPhysicalDev().createDevice(
				vk::DeviceCreateInfo()
				.setQueueCreateInfoCount(static_cast<uint32_t>(deviceQueueCreateInfos.size()))
				.setPQueueCreateInfos(deviceQueueCreateInfos.data())
				.setEnabledExtensionCount(static_cast<uint32_t>(deviceExtensions.size()))
				.setPpEnabledExtensionNames(deviceExtensions.data())
			)
		);

		auto findPresentQueue = std::ranges::find_if(indices.uniqueIndices, [&](uint32_t index) {
			return index == indices.PresentFamily;
			});

		if (findPresentQueue == indices.uniqueIndices.end())
		{
			indices.uniqueIndices.reserve(indices.uniqueIndices.size() + 1);
			indices.uniqueIndices.emplace_back(indices.PresentFamily);
		}
	}

	void VulkanContext::SetupSwapchain(SDL_Window& windowHandle, const QueueFamilyIndices& queueFamilyIndices)
	{
		(void)windowHandle;

		vk::PhysicalDeviceSurfaceInfo2KHR surfaceInfo{};
		std::vector<vk::SurfaceFormatKHR> surfaceFormats = VulkanCore::GetConstInstance().GetPhysicalDev().getSurfaceFormatsKHR(VulkanCore::GetConstInstance().GetSurface());
		if (surfaceFormats.empty())
			Log<LogLevel::Critical>("Failed to find any surface formats for the swapchain");
		
		vk::Format format = (surfaceFormats[0].format == vk::Format::eUndefined) ? vk::Format::eR8G8B8A8Unorm : surfaceFormats[0].format;
		vk::ColorSpaceKHR colorSpace = surfaceFormats[0].colorSpace;

		Log<LogLevel::Debug>("Vulkan Swapchain Surface Format selected:");
		Log<LogLevel::Debug>(" Format: {}", vk::to_string(format));
		Log<LogLevel::Debug>(" Color Space: {}", vk::to_string(colorSpace));

		vk::SurfaceCapabilities2KHR surfaceCapabilities = VulkanCore::GetConstInstance().GetPhysicalDev().getSurfaceCapabilities2KHR(VulkanCore::GetConstInstance().GetSurface());
		if (surfaceCapabilities.surfaceCapabilities.currentExtent.width == 0 || surfaceCapabilities.surfaceCapabilities.currentExtent.height == 0)
			Log<LogLevel::Critical>("Failed to get valid surface extents for the swapchain");

		vk::Extent2D swapchainExtent = surfaceCapabilities.surfaceCapabilities.currentExtent;
		Log<LogLevel::Debug>("Vulkan Swapchain Surface Capabilities:");
		Log<LogLevel::Debug>(" Current Extent: {}x{}", swapchainExtent.width, swapchainExtent.height);

		auto presentModeCompatibilities = VulkanCore::GetConstInstance().GetPhysicalDev().getSurfacePresentModesKHR(VulkanCore::GetConstInstance().GetSurface());

		if (presentModeCompatibilities.size() == 0)
			Log<LogLevel::Critical>("Failed to find any present modes for the swapchain");

		Log<LogLevel::Debug>("Vulkan Swapchain Present Modes supported:");
		for (const auto& presentMode : presentModeCompatibilities)
			Log<LogLevel::Debug>(" {}", vk::to_string(presentMode));

		vk::PresentModeKHR selectedPresentMode = vk::PresentModeKHR::eFifo; // guaranteed to be available
		// prefer mailbox present mode if available
		for (const auto& presentMode : presentModeCompatibilities)
			if (presentMode == vk::PresentModeKHR::eMailbox)
			{
				selectedPresentMode = vk::PresentModeKHR::eMailbox;
				break;
			}

		Log<LogLevel::Debug>("Vulkan Swapchain Present Mode selected: {}", vk::to_string(selectedPresentMode));

		vk::SurfaceTransformFlagBitsKHR preTransform = (surfaceCapabilities.surfaceCapabilities.supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity)
			? vk::SurfaceTransformFlagBitsKHR::eIdentity
			: surfaceCapabilities.surfaceCapabilities.currentTransform;

		vk::SwapchainCreateInfoKHR swapchainCreateInfo;
		swapchainCreateInfo
			.setSurface(VulkanCore::GetConstInstance().GetSurface())
			.setMinImageCount(surfaceCapabilities.surfaceCapabilities.minImageCount + 1)
			.setImageFormat(format)
			.setImageColorSpace(colorSpace)
			.setImageExtent(swapchainExtent)
			.setImageArrayLayers(1)
			.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
			.setImageSharingMode(vk::SharingMode::eExclusive)
			.setPreTransform(preTransform)
			.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
			.setPresentMode(selectedPresentMode)
			.setClipped(VK_TRUE);

		if (queueFamilyIndices.uniqueIndices.size() > 1)
			swapchainCreateInfo
				.setImageSharingMode(vk::SharingMode::eConcurrent)
				.setQueueFamilyIndexCount(static_cast<uint32_t>(queueFamilyIndices.uniqueIndices.size()))
				.setPQueueFamilyIndices(queueFamilyIndices.uniqueIndices.data());

		VulkanCore::GetInstance().SetSwapchain(VulkanCore::GetConstInstance().GetDevice().createSwapchainKHR(swapchainCreateInfo));

		VulkanCore::GetInstance().SetSwapchainImages(
			VulkanCore::GetConstInstance().GetDevice().getSwapchainImagesKHR(VulkanCore::GetConstInstance().GetSwapchain())
		);

		Log<LogLevel::Debug>("Vulkan Swapchain created with {} images", VulkanCore::GetConstInstance().GetSwapchainImages().size());

		std::vector<vk::ImageView> swapchainImageViews;
		swapchainImageViews.reserve(VulkanCore::GetConstInstance().GetSwapchainImages().size());

		for (const auto& image : VulkanCore::GetConstInstance().GetSwapchainImages())
		{
			vk::ImageViewCreateInfo imageViewCreateInfo;
			imageViewCreateInfo
				.setImage(image)
				.setViewType(vk::ImageViewType::e2D)
				.setFormat(format)
				.setComponents(
					vk::ComponentMapping()
					.setR(vk::ComponentSwizzle::eIdentity)
					.setG(vk::ComponentSwizzle::eIdentity)
					.setB(vk::ComponentSwizzle::eIdentity)
					.setA(vk::ComponentSwizzle::eIdentity)
				).setSubresourceRange(
					vk::ImageSubresourceRange()
					.setAspectMask(vk::ImageAspectFlagBits::eColor)
					.setBaseMipLevel(0)
					.setLevelCount(1)
					.setBaseArrayLayer(0)
					.setLayerCount(1)
				);
			swapchainImageViews.emplace_back(VulkanCore::GetConstInstance().GetDevice().createImageView(imageViewCreateInfo));
		}

		Log<LogLevel::NewLine>();
	}
}