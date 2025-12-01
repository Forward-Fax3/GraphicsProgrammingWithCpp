#include <limits>
#include <ranges>
#include <mutex>
#include <map>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_to_string.hpp>
#include <SDL3/SDL_vulkan.h>

#include "Application.hpp"
#include "VulkanContext.hpp"
#include "VulkanCore.hpp"
#include "Log.hpp"


#ifndef DIST
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

	if (pCallbackData == nullptr)
	{
		std::lock_guard lock(s_Mutex);

		if (s_MessagesLogged.empty())
			return vk::False;

		PrintVulkanDebugMessages(lastMessageSeverity, s_MessagesLogged);
		s_MessagesLogged.clear();
		lastMessageID = std::numeric_limits<int32_t>::max();
		lastMessageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT();
		firstMessage = true;
		return vk::False;
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

		if (messageSeverity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eError)
		{
			PrintVulkanDebugMessages(messageSeverity, s_MessagesLogged);
			s_MessagesLogged.clear();
		}
		else
		{
			lastMessageID = pCallbackData->messageIdNumber;
			lastMessageSeverity = messageSeverity;
			firstMessage = false;
		}
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

	return vk::False;
}
#endif

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
			SelectPhysicalDevice();
			FindQueueFamilies();
			CreateLogicalDevice();
			GetAndStoreGlobalQueueFamilies();
			CreateSwapchain();
			CreateRenderPass();
			CreateFramebuffers(1920, 1080);
			CreateCommandPools();
		}
		catch (const vk::SystemError& err)
		{
			Log<LogLevel::Critical>("Vulkan Context initialization failed: {}", err.what());
		}
		catch (const std::exception& ex)
		{
			Log<LogLevel::Critical>("Vulkan Context initialization failed: {}", ex.what());
		}

		vk::Result result = VulkanCore::GetInstance().IncrementCurrentFrameIndex();

		if (result != vk::Result::eSuccess)
			Log<LogLevel::Critical>("VulkanContext::VulkanContext: Failed to acquire initial swapchain image!");

#ifndef DIST 
		// flush any logged messages during initialization
		debugMessageFunc(
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo,
			vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral,
			nullptr,
			nullptr
		);
#endif
	}

	VulkanContext::~VulkanContext()
	{
		WaitForIdle();
		auto& vkCore = VulkanCore::GetConstInstance();

		VulkanCore::GetInstance().ResetRenderPassDatas();

		vkCore.GetDevice().destroySemaphore(vkCore.GetImageAvailableSemaphore());

		vkCore.GetDevice().destroyCommandPool(vkCore.GetGraphicsCommandPool());
		vkCore.GetDevice().destroyCommandPool(vkCore.GetComputeCommandPool());
		vkCore.GetDevice().destroyCommandPool(vkCore.GetTransferCommandPool());

		DestroySwapchain();

		vkCore.GetDevice().destroyRenderPass(vkCore.GetRenderPass());
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

	void VulkanContext::FinishRender()
	{
		auto& vkCore = VulkanCore::GetInstance();
		auto [temp, _] = vkCore.GetRenderPassDatas();
		std::vector<std::shared_ptr<VulkanRenderPass>>& renderPassData = temp.get();

		for (const auto& renderPassData : renderPassData)
			while (vkCore.GetDevice().waitForFences(renderPassData->GetFence(), vk::True, 16'666) == vk::Result::eTimeout);

		auto indices = static_cast<uint32_t>(vkCore.GetCurrentFrameIndex());
		auto result = VulkanCore::GetConstInstance().GetPresentQueue().presentKHR(
			vk::PresentInfoKHR()
			.setSwapchains(vkCore.GetSwapchain())
			.setSwapchainCount(1)
			.setPImageIndices(&indices)
		);

		size_t retryCount = 0;
		while (result != vk::Result::eSuccess)
		{
			if (retryCount++ >= 3)
				Log<LogLevel::Critical>("VulkanContext::FinishRender: Failed to present image after 3 retries.");

			RecreateSwapchain();

			result = VulkanCore::GetConstInstance().GetPresentQueue().presentKHR(
				vk::PresentInfoKHR()
				.setSwapchains(vkCore.GetSwapchain())
				.setSwapchainCount(1)
				.setPImageIndices(&indices)
			);

			m_RenderPassNeedsRecreating = true;
		}

		renderPassData.clear();
	}

	void VulkanContext::SwapPresentImage()
	{
		vk::Result result = VulkanCore::GetInstance().IncrementCurrentFrameIndex();
		size_t retryCount = 0;

		while (result != vk::Result::eSuccess)
		{
			if (retryCount++ >= 3)
				Log<LogLevel::Critical>("VulkanContext::SwapPresentImage: Failed to acquire next image after 3 retries.");

			RecreateSwapchain();
			result = VulkanCore::GetInstance().IncrementCurrentFrameIndex();
			m_RenderPassNeedsRecreating = true;
		}
	}

#ifndef DIST
	void VulkanContext::FlushValidationMessages()
	{
		debugMessageFunc(
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo,
			vk::DebugUtilsMessageTypeFlagsEXT(),
			nullptr,
			nullptr
		);
	}
#endif

	void VulkanContext::WaitForIdle()
	{
		VulkanCore::GetConstInstance().GetDevice().waitIdle();
	}

	void VulkanContext::AddRenderPassData(const std::shared_ptr<RenderPassData>& renderPassData)
	{
		VulkanCore::GetInstance().AddRenderPassData(renderPassData);
	}

	void VulkanContext::StartInstance()
	{
		std::vector<const char*> extentions;
		{
			uint32_t numberOfSDLExtensions = 0;
			const auto extentionsTemp = SDL_Vulkan_GetInstanceExtensions(&numberOfSDLExtensions);

			if constexpr (!IsDistributionMode()) // +3 for debug utils and get physical device properties 2 and surface extensions
				extentions.reserve(static_cast<size_t>(numberOfSDLExtensions) + 3);
			else // +2 for get physical device properties 2 and surface extensions
				extentions.reserve(static_cast<size_t>(numberOfSDLExtensions) + 2);

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

		vk::ApplicationInfo appInfo;
		appInfo
			.setPApplicationName("OOPWithCpp Application")
			.setApplicationVersion(VK_MAKE_VERSION(0, 0, 1))
			.setPEngineName("OOPWithCpp Engine")
			.setEngineVersion(VK_MAKE_VERSION(0, 0, 1))
			.setApiVersion(g_VulkanVersion);

		createInfo
			.setEnabledExtensionCount(static_cast<uint32_t>(extentions.size()))
			.setPpEnabledExtensionNames(extentions.data())
			.setPApplicationInfo(&appInfo);

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

	void VulkanContext::SelectPhysicalDevice()
	{
		const std::vector<vk::PhysicalDevice> physicalDevices = VulkanCore::GetConstInstance().GetVKInstance().enumeratePhysicalDevices();

		if (physicalDevices.empty())
			Log<LogLevel::Critical>("Failed to find GPUs with Vulkan support");

		uint32_t highestScore = 0;
		for (const auto& device : physicalDevices)
		{
			auto [isSuitable, score] = IsPhysicalDeviceSuitable(device);
			if (isSuitable && score > highestScore)
			{
				highestScore = score;
				VulkanCore::GetInstance().SetPhysicalDevice(device);
			}
		}

		if (!VulkanCore::GetConstInstance().GetPhysicalDev())
			Log<LogLevel::Critical>("Failed to find a suitable GPU");
		else
		{
			auto deviceProperties = VulkanCore::GetConstInstance().GetPhysicalDev().getProperties();

			Log<LogLevel::Debug>("Selected GPU: {} (ID: {}, GPU Type: {})",
				deviceProperties.deviceName.data(),
				deviceProperties.deviceID,
				vk::to_string(deviceProperties.deviceType));
			Log<LogLevel::NewLine>();
		}
	}

	std::pair<bool, uint32_t> VulkanContext::IsPhysicalDeviceSuitable(const vk::PhysicalDevice& device)
	{
		uint32_t score = 0;
		auto deviceProperties = device.getProperties2();
		auto supportedExtensions = device.enumerateDeviceExtensionProperties();

		if (deviceProperties.properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
			score += 1000;

		score += deviceProperties.properties.limits.maxImageDimension2D;

		if (!IsExtentionAvailable(supportedExtensions, vk::KHRSwapchainExtensionName))
			return { false, 0 };

		if (!IsExtentionAvailable(supportedExtensions, vk::KHRMaintenance1ExtensionName))
			return { false, 0 };

		if (!IsExtentionAvailable(supportedExtensions, vk::KHRShaderDrawParametersExtensionName))
			return { false, 0 };

		return { true, score };
	}

	void VulkanContext::FindQueueFamilies()
	{
		auto queueFamilies = VulkanCore::GetConstInstance().GetPhysicalDev().getQueueFamilyProperties();
		constexpr uint32_t indexMax = std::numeric_limits<uint32_t>::max();

		for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilies.size()); i++) // try to find all queue families in one loop and have different queue families if possible
		{
			if (m_QueueFamilyIndices.PresentFamily == indexMax &&
				VulkanCore::GetConstInstance().GetPhysicalDev().getSurfaceSupportKHR(i, VulkanCore::GetConstInstance().GetSurface()))
				m_QueueFamilyIndices.PresentFamily = i;

			if (m_QueueFamilyIndices.GraphicsFamily == indexMax && (queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics) == vk::QueueFlagBits::eGraphics)
				m_QueueFamilyIndices.GraphicsFamily = i;

			// prefer a dedicated compute queue family
			else if (m_QueueFamilyIndices.ComputeFamily == indexMax && (queueFamilies[i].queueFlags & vk::QueueFlagBits::eCompute) == vk::QueueFlagBits::eCompute)
				m_QueueFamilyIndices.ComputeFamily = i;

			// prefer a dedicated transfer queue family
			else if (m_QueueFamilyIndices.TransferFamily == indexMax && (queueFamilies[i].queueFlags & vk::QueueFlagBits::eTransfer) == vk::QueueFlagBits::eTransfer
				&& (queueFamilies[i].queueFlags & vk::QueueFlags(vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute)) == vk::QueueFlags(0))
				m_QueueFamilyIndices.TransferFamily = i;

			// break early if all found
			if (m_QueueFamilyIndices.FoundAll())
				break;
		}

		CheckQueueFamilyValidity(queueFamilies);

		Log<LogLevel::Debug>("Vulkan Queue Families found:");
		Log<LogLevel::Debug>(" Present Queue Family Index: {}", m_QueueFamilyIndices.PresentFamily);
		Log<LogLevel::Debug>(" Graphics Queue Family Index: {}", m_QueueFamilyIndices.GraphicsFamily);
		Log<LogLevel::Debug>(" Compute Queue Family Index: {}", m_QueueFamilyIndices.ComputeFamily);
		Log<LogLevel::Debug>(" Transfer Queue Family Index: {}", m_QueueFamilyIndices.TransferFamily);
		Log<LogLevel::NewLine>();
	}

	void VulkanContext::CheckQueueFamilyValidity(const std::vector<vk::QueueFamilyProperties> queueFamilies)
	{
		constexpr uint32_t indexMax = std::numeric_limits<uint32_t>::max();

		if (m_QueueFamilyIndices.PresentFamily == indexMax)
			Log<LogLevel::Critical>("Failed to find a valid present queue family index");
		else if (m_QueueFamilyIndices.GraphicsFamily == indexMax)
			Log<LogLevel::Critical>("Failed to find a valid graphics queue family index");
		else if (m_QueueFamilyIndices.ComputeFamily == indexMax) // check if compute queue family is found, otherwise find any even if it shares with others
		{
			Log<LogLevel::Warn>("Failed to find a unique compute queue family index, any compute queue will be searched for now");

			for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilies.size()); i++)
				if ((queueFamilies[i].queueFlags & vk::QueueFlagBits::eCompute) == vk::QueueFlagBits::eCompute)
				{
					m_QueueFamilyIndices.ComputeFamily = i;
					break;
				}
		}

		if (m_QueueFamilyIndices.ComputeFamily == indexMax)
			Log<LogLevel::Critical>("Failed to find a valid compute queue family index");
		else if (m_QueueFamilyIndices.TransferFamily == indexMax) // check if transfer queue family is found, otherwise find any even if it shares with others
		{
			Log<LogLevel::Trace>("Failed to find a unique transfer queue family index, any transfer queue will be searched for now");

			for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilies.size()); i++)
				if ((queueFamilies[i].queueFlags & vk::QueueFlagBits::eTransfer) == vk::QueueFlagBits::eTransfer)
				{
					m_QueueFamilyIndices.TransferFamily = i;
					break;
				}
		}

		// if still not found, use graphics queue family
		if (m_QueueFamilyIndices.TransferFamily == indexMax)
		{
			Log<LogLevel::Trace>("Failed to find a transfer queue bit in any family index, graphics queue family will be used implicitly instead");
			m_QueueFamilyIndices.TransferFamily = m_QueueFamilyIndices.GraphicsFamily;
		}
	}

	void VulkanContext::CreateLogicalDevice()
	{
		const std::array<const char*, 3> deviceExtensions = {
			vk::KHRSwapchainExtensionName,
			vk::KHRMaintenance1ExtensionName,
			vk::KHRShaderDrawParametersExtensionName
		};

		std::map<uint32_t, std::pair<uint32_t, std::vector<float>>> uniqueQueueFamiliesMap;

		for (const auto& index : { m_QueueFamilyIndices.GraphicsFamily, m_QueueFamilyIndices.ComputeFamily, m_QueueFamilyIndices.TransferFamily })
		{
			if (!uniqueQueueFamiliesMap.contains(index))
				uniqueQueueFamiliesMap[index].first = 1;
			else
				uniqueQueueFamiliesMap[index].first++;

			uniqueQueueFamiliesMap[index].second.push_back(1.0f);
		}

		// ensure present family is also included if it is in its own queue family
		if (!uniqueQueueFamiliesMap.contains(m_QueueFamilyIndices.PresentFamily))
		{
			uniqueQueueFamiliesMap[m_QueueFamilyIndices.PresentFamily].first = 1;
			uniqueQueueFamiliesMap[m_QueueFamilyIndices.PresentFamily].second.push_back(1.0f);
		}

		std::vector<vk::DeviceQueueCreateInfo> deviceQueueCreateInfos;
		deviceQueueCreateInfos.reserve(uniqueQueueFamiliesMap.size());

		for (const auto& [familyIndex, familyData] : uniqueQueueFamiliesMap)
			deviceQueueCreateInfos.emplace_back(
				vk::DeviceQueueCreateInfo()
				.setQueueFamilyIndex(familyIndex)
				.setQueueCount(familyData.first)
				.setPQueuePriorities(familyData.second.data())
			);

		VulkanCore::GetInstance().SetDevice(
			VulkanCore::GetConstInstance().GetPhysicalDev().createDevice(
				vk::DeviceCreateInfo()
				.setQueueCreateInfoCount(static_cast<uint32_t>(deviceQueueCreateInfos.size()))
				.setPQueueCreateInfos(deviceQueueCreateInfos.data())
				.setEnabledExtensionCount(static_cast<uint32_t>(deviceExtensions.size()))
				.setPpEnabledExtensionNames(deviceExtensions.data())
			)
		);

		m_QueueFamilyIndices.uniqueIndices.reserve(uniqueQueueFamiliesMap.size());
		for (const auto& familyIndex : std::views::keys(uniqueQueueFamiliesMap))
			m_QueueFamilyIndices.uniqueIndices.emplace_back(familyIndex);
	}

	void VulkanContext::GetAndStoreGlobalQueueFamilies() const
	{
		std::map<uint32_t, uint32_t> queueFamilyUsageCount;

		auto l_getQueue = [&](uint32_t familyIndex) -> vk::Queue {
			if (!queueFamilyUsageCount.contains(familyIndex))
				queueFamilyUsageCount[familyIndex] = 0;

			return VulkanCore::GetConstInstance().GetDevice().getQueue(familyIndex, queueFamilyUsageCount[familyIndex]++);
		};
		
		VulkanCore::GetInstance().SetGraphicsQueue(l_getQueue(m_QueueFamilyIndices.GraphicsFamily));
		VulkanCore::GetInstance().SetComputeQueue(l_getQueue(m_QueueFamilyIndices.ComputeFamily));
		VulkanCore::GetInstance().SetTransferQueue(l_getQueue(m_QueueFamilyIndices.TransferFamily));

		if (queueFamilyUsageCount.contains(m_QueueFamilyIndices.PresentFamily))
			VulkanCore::GetInstance().SetPresentQueue(
				VulkanCore::GetConstInstance().GetDevice().getQueue(m_QueueFamilyIndices.PresentFamily, 0)
			);
		else
			VulkanCore::GetInstance().SetPresentQueue(l_getQueue(m_QueueFamilyIndices.PresentFamily));
	}

	void VulkanContext::CreateSwapchain()
	{
		vk::PhysicalDeviceSurfaceInfo2KHR surfaceInfo{};
		std::vector<vk::SurfaceFormatKHR> surfaceFormats = VulkanCore::GetConstInstance().GetPhysicalDev().getSurfaceFormatsKHR(VulkanCore::GetConstInstance().GetSurface());
		if (surfaceFormats.empty())
			Log<LogLevel::Critical>("Failed to find any surface formats for the swapchain");

		VulkanCore::GetInstance().SetSwapchainImageFormat((surfaceFormats[0].format == vk::Format::eUndefined) ? vk::Format::eR8G8B8A8Unorm : surfaceFormats[0].format);
		vk::ColorSpaceKHR colorSpace = surfaceFormats[0].colorSpace;

		Log<LogLevel::Debug>("Vulkan Swapchain Surface Format selected:");
		Log<LogLevel::Debug>(" Format: {}", vk::to_string(VulkanCore::GetConstInstance().GetSwapchainImageFormat()));
		Log<LogLevel::Debug>(" Color Space: {}", vk::to_string(colorSpace));

		vk::SurfaceCapabilities2KHR surfaceCapabilities = VulkanCore::GetConstInstance().GetPhysicalDev().getSurfaceCapabilities2KHR(VulkanCore::GetConstInstance().GetSurface());
		if (surfaceCapabilities.surfaceCapabilities.currentExtent.width == 0 || surfaceCapabilities.surfaceCapabilities.currentExtent.height == 0)
			Log<LogLevel::Critical>("Failed to get valid surface extents for the swapchain");

		// SDL gives the actual window size, so we can use that directly
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
			.setImageFormat(VulkanCore::GetConstInstance().GetSwapchainImageFormat())
			.setImageColorSpace(colorSpace)
			.setImageExtent(swapchainExtent)
			.setImageArrayLayers(1)
			.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
			.setImageSharingMode(vk::SharingMode::eExclusive)
			.setPreTransform(preTransform)
			.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
			.setPresentMode(selectedPresentMode)
			.setClipped(vk::True);

		if (m_QueueFamilyIndices.uniqueIndices.size() > 1)
			swapchainCreateInfo
				.setImageSharingMode(vk::SharingMode::eConcurrent)
				.setQueueFamilyIndexCount(static_cast<uint32_t>(m_QueueFamilyIndices.uniqueIndices.size()))
				.setPQueueFamilyIndices(m_QueueFamilyIndices.uniqueIndices.data());

		VulkanCore::GetInstance().SetSwapchain(VulkanCore::GetConstInstance().GetDevice().createSwapchainKHR(swapchainCreateInfo));

		VulkanCore::GetInstance().SetSwapchainImages(
			VulkanCore::GetConstInstance().GetDevice().getSwapchainImagesKHR(VulkanCore::GetConstInstance().GetSwapchain())
		);

		Log<LogLevel::Debug>("Vulkan Swapchain created with {} images", VulkanCore::GetConstInstance().GetSwapchainImages().size());

		std::vector<vk::ImageView>& swapchainImageViews = VulkanCore::GetInstance().GetSwapchainImageViews();

		for (const auto& image : VulkanCore::GetConstInstance().GetSwapchainImages())
		{
			vk::ImageViewCreateInfo imageViewCreateInfo;
			imageViewCreateInfo
				.setImage(image)
				.setViewType(vk::ImageViewType::e2D)
				.setFormat(VulkanCore::GetConstInstance().GetSwapchainImageFormat())
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

	void VulkanContext::CreateFramebuffers(uint32_t width, uint32_t height)
	{
		std::vector<vk::Framebuffer>& swapchainFramebuffers = VulkanCore::GetInstance().GetSwapchainFramebuffers();
		swapchainFramebuffers.reserve(VulkanCore::GetConstInstance().GetSwapchainImageViews().size());

		for (const auto& imageView : VulkanCore::GetConstInstance().GetSwapchainImageViews())
			swapchainFramebuffers.emplace_back(VulkanCore::GetConstInstance().GetDevice().createFramebuffer(
				vk::FramebufferCreateInfo()
					.setRenderPass(VulkanCore::GetConstInstance().GetRenderPass())
					.setAttachmentCount(1)
					.setPAttachments(&imageView)
					.setWidth(width)
					.setHeight(height)
					.setLayers(1)
				)
			);
	}

	void VulkanContext::CreateRenderPass()
	{
		vk::AttachmentDescription colorAttachment{};
		colorAttachment
			.setFormat(VulkanCore::GetConstInstance().GetSwapchainImageFormat())
			.setSamples(vk::SampleCountFlagBits::e1)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

		vk::AttachmentReference colorAttachmentRef{};
		colorAttachmentRef
			.setAttachment(0)
			.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

		vk::SubpassDescription subpass{};
		subpass
			.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
			.setColorAttachmentCount(1)
			.setPColorAttachments(&colorAttachmentRef);

		vk::RenderPassCreateInfo renderPassInfo{};
		renderPassInfo
			.setAttachmentCount(1)
			.setPAttachments(&colorAttachment)
			.setSubpassCount(1)
			.setPSubpasses(&subpass);

		VulkanCore::GetInstance().SetRenderPass(VulkanCore::GetConstInstance().GetDevice().createRenderPass(renderPassInfo));
	}

	void VulkanContext::CreateCommandPools() const
	{
		VulkanCore::GetInstance().SetGraphicsCommandPool(
			VulkanCore::GetConstInstance().GetDevice().createCommandPool(
				vk::CommandPoolCreateInfo()
				.setQueueFamilyIndex(m_QueueFamilyIndices.GraphicsFamily)
				.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
			)
		);

		VulkanCore::GetInstance().SetComputeCommandPool(
			VulkanCore::GetConstInstance().GetDevice().createCommandPool(
				vk::CommandPoolCreateInfo()
				.setQueueFamilyIndex(m_QueueFamilyIndices.ComputeFamily)
				.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
			)
		);

		VulkanCore::GetInstance().SetTransferCommandPool(
			VulkanCore::GetConstInstance().GetDevice().createCommandPool(
				vk::CommandPoolCreateInfo()
				.setQueueFamilyIndex(m_QueueFamilyIndices.TransferFamily)
				.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
			)
		);
	}

	void VulkanContext::DestroySwapchain()
	{
		const auto& vkCore = VulkanCore::GetConstInstance();

		for (const auto& frameBuffer : vkCore.GetSwapchainFramebuffers())
			vkCore.GetDevice().destroyFramebuffer(frameBuffer);

		for (const auto& imageView : vkCore.GetSwapchainImageViews())
			vkCore.GetDevice().destroyImageView(imageView);

		vkCore.GetDevice().destroySwapchainKHR(vkCore.GetSwapchain());
	}

	void VulkanContext::RecreateSwapchain()
	{
		const auto& app = Application::GetConstInstance();
		auto& vkCore = VulkanCore::GetInstance();
		WaitForIdle();
		DestroySwapchain();
		vkCore.GetSwapchainImages().clear();
		vkCore.GetSwapchainImageViews().clear();
		vkCore.GetSwapchainFramebuffers().clear();
		CreateSwapchain();
		CreateFramebuffers(app.GetWindowWidth(), app.GetWindowHeight());
	}
}
