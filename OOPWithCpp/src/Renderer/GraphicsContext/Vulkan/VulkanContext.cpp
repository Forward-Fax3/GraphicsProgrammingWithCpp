#include <limits>
#include <ranges>
#include <mutex>
#include <map>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_to_string.hpp>
#include <SDL3/SDL_vulkan.h>
#define IMGUI_IMPL_VULKAN_HAS_DYNAMIC_RENDERING
#include <backends/imgui_impl_vulkan.h>

#include "Application.hpp"
#include "VulkanContext.hpp"
#include "VulkanCore.hpp"
#include "Log.hpp"


// TODO: Change to Dynamic rendering

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
	std::array<const char*, 5> l_DeviceExtensions = {
			vk::KHRSwapchainExtensionName,
			vk::KHRMaintenance1ExtensionName,
			vk::KHRShaderDrawParametersExtensionName,
			vk::KHRDynamicRenderingExtensionName,
			vk::KHRSynchronization2ExtensionName
	};

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
			CreateCommandPools();
			WriteCommandBuffers();
		}
		catch (const vk::SystemError& err)
		{
			Log<LogLevel::Critical>("Vulkan Context initialization failed: {}", err.what());
		}
		catch (const std::exception& ex)
		{
			Log<LogLevel::Critical>("Vulkan Context initialization failed: {}", ex.what());
		}

		SwapPresentImage();

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

		VulkanCore::GetInstance().DestroySemaphores();

		vkCore.GetDevice().destroyCommandPool(vkCore.GetGraphicsCommandPool());
		vkCore.GetDevice().destroyCommandPool(vkCore.GetComputeCommandPool());
		vkCore.GetDevice().destroyCommandPool(vkCore.GetTransferCommandPool());
		vkCore.GetDevice().destroyCommandPool(vkCore.GetDynamicGraphicsCommandPool());
		vkCore.GetDevice().destroyCommandPool(vkCore.GetDynamicComputeCommandPool());
		vkCore.GetDevice().destroyCommandPool(vkCore.GetDynamicTransferCommandPool());

		DestroySwapchain();

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
		m_RenderPassNeedsRecreating = false;

		auto& vkCore = VulkanCore::GetInstance();

		{
			auto [temp, _] = vkCore.GetRenderPassDatas();
			std::vector<std::shared_ptr<VulkanRenderPass>>& renderPassDatas = temp.get();

			for (const auto& renderPassData : renderPassDatas)
				while (vkCore.GetDevice().waitForFences(renderPassData->GetFence(), vk::True, 16'666) == vk::Result::eTimeout);

			renderPassDatas.clear();
		}

		if (!m_IsMinimized)
		{
			std::array<std::string_view, 2> semaphoreNames = { "ImGuiLayer" , "RenderFinished" };
			auto semaphores = vkCore.GetSemaphoresFromNames(semaphoreNames);

			vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eBottomOfPipe);
			vk::SubmitInfo submitInfo = vk::SubmitInfo()
				.setWaitSemaphores(semaphores[0])
				.setSignalSemaphores(semaphores[1])
				.setCommandBuffers(m_EndRenderCmdBuf[vkCore.GetCurrentFrameIndex()])
				.setWaitDstStageMask(waitDestinationStageMask);

			vkCore.GetGraphicsQueue().submit(submitInfo, VK_NULL_HANDLE);

			auto indices = static_cast<uint32_t>(vkCore.GetCurrentFrameIndex());
			auto result = VulkanCore::GetConstInstance().GetPresentQueue().presentKHR(
				vk::PresentInfoKHR()
				.setSwapchains(vkCore.GetSwapchain())
				.setSwapchainCount(1)
				.setImageIndices(indices)
				.setWaitSemaphores(semaphores[1])
			);

			(void)result;

//			size_t retryCount = 0;
//			while (result == vk::Result::eSuboptimalKHR || result == vk::Result::eErrorOutOfDateKHR)
//			{
//				if (retryCount++ >= 3)
//					Log<LogLevel::Critical>("VulkanContext::FinishRender: Failed to present image after 3 retries.");
//	
//				RecreateSwapchain();
//	
//				result = VulkanCore::GetConstInstance().GetPresentQueue().presentKHR(
//					vk::PresentInfoKHR()
//					.setSwapchains(vkCore.GetSwapchain())
//					.setSwapchainCount(1)
//					.setImageIndices(indices)
//				);
//			}
		}
	}

	void VulkanContext::SwapPresentImage()
	{
		auto& vkCore = VulkanCore::GetInstance();
		const auto [_, imageAcquired] = vkCore.IncrementCurrentFrameIndex();

//		size_t retryCount = 0;
//
//		while (result == vk::Result::eSuboptimalKHR || result == vk::Result::eErrorOutOfDateKHR)
//		{
//			if (retryCount++ >= 3)
//				Log<LogLevel::Critical>("VulkanContext::SwapPresentImage: Failed to acquire next image after 3 retries.");
//
//			RecreateSwapchain();
//			result = vkCore.IncrementCurrentFrameIndex();
//		}

		std::array<std::string_view, 1> imageReadyName = { "ImageReady" };
		auto imageReady = vkCore.GetSemaphoresFromNames(imageReadyName)[0];

		vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
		vk::SubmitInfo submitInfo = vk::SubmitInfo()
			.setWaitSemaphores(imageAcquired)
			.setSignalSemaphores(imageReady)
			.setCommandBuffers(m_BeginRenderCmdBuf[vkCore.GetCurrentFrameIndex()])
			.setWaitDstStageMask(waitDestinationStageMask);

		vkCore.GetGraphicsQueue().submit(submitInfo, VK_NULL_HANDLE);

		vkCore.GetRenderPassDatas().first.get().clear();
	}

	void VulkanContext::WaitForIdle()
	{
		VulkanCore::GetConstInstance().GetDevice().waitIdle();
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

		auto instanceExtertionProperties = vk::enumerateInstanceExtensionProperties();

		extentions.emplace_back(vk::KHRGetPhysicalDeviceProperties2ExtensionName);
		extentions.emplace_back(vk::KHRGetSurfaceCapabilities2ExtensionName);

		vk::InstanceCreateInfo createInfo;
		
		std::vector<const char*> validationLayers;
		if constexpr (!IsDistributionMode())
		{
				extentions.emplace_back(vk::EXTDebugUtilsExtensionName);

				validationLayers.push_back("VK_LAYER_KHRONOS_validation");
				createInfo.setEnabledLayerCount(static_cast<uint32_t>(validationLayers.size()));
				createInfo.setPpEnabledLayerNames(validationLayers.data());
		}

		const auto [extensionsAvailable, missingExtension] = IsExtentionAvailable(instanceExtertionProperties, extentions);

		if (!extensionsAvailable)
			Log<LogLevel::Critical>("Vulkan instance is missing required extension: {}", missingExtension);

		constexpr vk::ApplicationInfo appInfo = vk::ApplicationInfo()
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

		const auto [extensionsAvailable, missingExtension] = IsExtentionAvailable(supportedExtensions, l_DeviceExtensions);
		if (!extensionsAvailable)
		{
			Log<LogLevel::Warn>("Physical device {} is missing required extension: {}",
				deviceProperties.properties.deviceName.data(),
				missingExtension);
			return { false, 0 };
		}

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

	void VulkanContext::CreateLogicalDevice()
	{
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

		vk::PhysicalDeviceDescriptorIndexingFeatures descriptorIndexingFeature = vk::PhysicalDeviceDescriptorIndexingFeatures()
			.setPNext(nullptr)
			.setRuntimeDescriptorArray(vk::True)
			.setDescriptorBindingPartiallyBound(vk::True)
			.setDescriptorBindingVariableDescriptorCount(vk::True)
			.setShaderSampledImageArrayNonUniformIndexing(vk::True)
			.setShaderStorageBufferArrayNonUniformIndexing(vk::True)
			.setShaderUniformBufferArrayNonUniformIndexing(vk::True);

		vk::PhysicalDeviceSwapchainMaintenance1FeaturesKHR swapchainMaintenance1Feature = vk::PhysicalDeviceSwapchainMaintenance1FeaturesKHR()
			.setPNext(&descriptorIndexingFeature)
			.setSwapchainMaintenance1(vk::True);

		vk::PhysicalDeviceShaderObjectFeaturesEXT shaderObjectFeature = vk::PhysicalDeviceShaderObjectFeaturesEXT()
			.setPNext(&swapchainMaintenance1Feature)
			.setShaderObject(vk::True);

		vk::PhysicalDeviceSynchronization2Features synchronization2Feature = vk::PhysicalDeviceSynchronization2Features()
			.setPNext(shaderObjectFeature)
			.setSynchronization2(vk::True);

		const vk::PhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeature = vk::PhysicalDeviceDynamicRenderingFeatures()
			.setPNext(&synchronization2Feature)
			.setDynamicRendering(vk::True);

		VulkanCore::GetInstance().SetDevice(
			VulkanCore::GetConstInstance().GetPhysicalDev().createDevice(
				vk::DeviceCreateInfo()
				.setQueueCreateInfoCount(static_cast<uint32_t>(deviceQueueCreateInfos.size()))
				.setPQueueCreateInfos(deviceQueueCreateInfos.data())
				.setEnabledExtensionCount(static_cast<uint32_t>(l_DeviceExtensions.size()))
				.setPpEnabledExtensionNames(l_DeviceExtensions.data())
				.setPNext(&dynamicRenderingFeature)
			)
		);

		m_QueueFamilyIndices.uniqueIndices.reserve(uniqueQueueFamiliesMap.size());
		for (const auto& familyIndex : std::views::keys(uniqueQueueFamiliesMap))
			m_QueueFamilyIndices.uniqueIndices.emplace_back(familyIndex);
	}

	void VulkanContext::CreateSwapchain()
	{
		if (m_IsMinimized)
			return;

		auto& vkCore = VulkanCore::GetConstInstance();

		std::vector<vk::SurfaceFormatKHR> surfaceFormats = vkCore.GetPhysicalDev().getSurfaceFormatsKHR(vkCore.GetSurface());
		if (surfaceFormats.empty())
			Log<LogLevel::Critical>("Failed to find any surface formats for the swapchain");

		VulkanCore::GetInstance().SetSwapchainImageFormat((surfaceFormats[0].format == vk::Format::eUndefined) ? vk::Format::eR8G8B8A8Unorm : surfaceFormats[0].format);
		vk::ColorSpaceKHR colorSpace = surfaceFormats[0].colorSpace;

		Log<LogLevel::Debug>("Vulkan Swapchain Surface Format selected:");
		Log<LogLevel::Debug>(" Format: {}", vk::to_string(vkCore.GetSwapchainImageFormat()));
		Log<LogLevel::Debug>(" Color Space: {}", vk::to_string(colorSpace));

		vk::SurfaceCapabilities2KHR surfaceCapabilities = vkCore.GetPhysicalDev().getSurfaceCapabilities2KHR(vkCore.GetSurface());

		if (surfaceCapabilities.surfaceCapabilities.currentExtent.width == 0 || surfaceCapabilities.surfaceCapabilities.currentExtent.height == 0)
			Log<LogLevel::Critical>("Failed to get valid surface extents for the swapchain");

		// SDL gives the actual window size, so we can use that directly
		vk::Extent2D swapchainExtent = surfaceCapabilities.surfaceCapabilities.currentExtent;
		Log<LogLevel::Debug>("Vulkan Swapchain Surface Capabilities:");
		Log<LogLevel::Debug>(" Current Extent: {}x{}", swapchainExtent.width, swapchainExtent.height);

		auto presentModeCompatibilities = vkCore.GetPhysicalDev().getSurfacePresentModesKHR(vkCore.GetSurface());

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

		vk::SwapchainCreateInfoKHR swapchainCreateInfo = vk::SwapchainCreateInfoKHR()
			.setSurface(vkCore.GetSurface())
			.setMinImageCount(std::max(surfaceCapabilities.surfaceCapabilities.minImageCount, 3u))
			.setImageFormat(vkCore.GetSwapchainImageFormat())
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

		VulkanCore::GetInstance().SetSwapchain(vkCore.GetDevice().createSwapchainKHR(swapchainCreateInfo));

		VulkanCore::GetInstance().SetSwapchainImages(
			vkCore.GetDevice().getSwapchainImagesKHR(vkCore.GetSwapchain())
		);

		Log<LogLevel::Debug>("Vulkan Swapchain created with {} images", vkCore.GetSwapchainImages().size());

		std::vector<vk::ImageView>& swapchainImageViews = VulkanCore::GetInstance().GetSwapchainImageViews();

		for (const auto& image : vkCore.GetSwapchainImages())
		{
			vk::ImageViewCreateInfo imageViewCreateInfo = vk::ImageViewCreateInfo()
				.setImage(image)
				.setViewType(vk::ImageViewType::e2D)
				.setFormat(vkCore.GetSwapchainImageFormat())
				.setComponents(vk::ComponentMapping()
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
			swapchainImageViews.emplace_back(vkCore.GetDevice().createImageView(imageViewCreateInfo));
		}

		Log<LogLevel::NewLine>();
	}

	void VulkanContext::CreateCommandPools() const
	{
		VulkanCore::GetInstance().SetGraphicsCommandPool(
			VulkanCore::GetConstInstance().GetDevice().createCommandPool(
				vk::CommandPoolCreateInfo()
				.setQueueFamilyIndex(m_QueueFamilyIndices.GraphicsFamily)
			)
		);

		VulkanCore::GetInstance().SetComputeCommandPool(
			VulkanCore::GetConstInstance().GetDevice().createCommandPool(
				vk::CommandPoolCreateInfo()
				.setQueueFamilyIndex(m_QueueFamilyIndices.ComputeFamily)
			)
		);

		VulkanCore::GetInstance().SetTransferCommandPool(
			VulkanCore::GetConstInstance().GetDevice().createCommandPool(
				vk::CommandPoolCreateInfo()
				.setQueueFamilyIndex(m_QueueFamilyIndices.TransferFamily)
			)
		);

		VulkanCore::GetInstance().SetDynamicGraphicsCommandPool(
			VulkanCore::GetConstInstance().GetDevice().createCommandPool(
				vk::CommandPoolCreateInfo()
				.setQueueFamilyIndex(m_QueueFamilyIndices.GraphicsFamily)
				.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
			)
		);

		VulkanCore::GetInstance().SetDynamicComputeCommandPool(
			VulkanCore::GetConstInstance().GetDevice().createCommandPool(
				vk::CommandPoolCreateInfo()
				.setQueueFamilyIndex(m_QueueFamilyIndices.ComputeFamily)
				.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
			)
		);

		VulkanCore::GetInstance().SetDynamicTransferCommandPool(
			VulkanCore::GetConstInstance().GetDevice().createCommandPool(
				vk::CommandPoolCreateInfo()
				.setQueueFamilyIndex(m_QueueFamilyIndices.TransferFamily)
				.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
			)
		);

		VulkanCore::GetInstance().SetupSemaphores();
	}

	void VulkanContext::WriteCommandBuffers()
	{
		auto& vkCore = VulkanCore::GetConstInstance();
		m_BeginRenderCmdBuf = vkCore.GetGraphicsCommandBuffer();
		m_EndRenderCmdBuf = vkCore.GetGraphicsCommandBuffer();

		vk::ClearValue clearColour(vk::ClearColorValue(std::array<float, 4>{0.0, 0.0, 0.0, 1.0}));

		auto windowSizeExtent = vkCore.GetPhysicalDev().getSurfaceCapabilitiesKHR(vkCore.GetSurface()).currentExtent;
		vk::Rect2D rect(vk::Offset2D(0, 0),
			windowSizeExtent
		);

		for (size_t i = 0; i != vkCore.GetSwapchainImageViews().size(); i++)
		{
			const auto& cmdBuf = m_BeginRenderCmdBuf[i];

			cmdBuf.begin(vk::CommandBufferBeginInfo());
			cmdBuf.pipelineBarrier2(vk::DependencyInfo()
				.setImageMemoryBarriers(vk::ImageMemoryBarrier2()
					.setSrcStageMask(vk::PipelineStageFlagBits2::eNone)
					.setDstStageMask(vk::PipelineStageFlagBits2::eColorAttachmentOutput)
					.setSrcAccessMask(vk::AccessFlagBits2::eNone)
					.setDstAccessMask(vk::AccessFlagBits2::eColorAttachmentWrite)
					.setOldLayout(vk::ImageLayout::eUndefined)
					.setNewLayout(vk::ImageLayout::eColorAttachmentOptimal)
					.setImage(vkCore.GetSwapchainImages()[i])
					.setSubresourceRange(vk::ImageSubresourceRange()
						.setAspectMask(vk::ImageAspectFlagBits::eColor)
						.setBaseMipLevel(0)
						.setLevelCount(1)
						.setBaseArrayLayer(0)
						.setLayerCount(1)
					)
				)
			);

			cmdBuf.beginRendering(vk::RenderingInfo()
				.setRenderArea(rect)
				.setLayerCount(1)
				.setColorAttachmentCount(1)
				.setPColorAttachments(&vk::RenderingAttachmentInfo()
					.setImageView(vkCore.GetSwapchainImageViews()[i])
					.setImageLayout(vk::ImageLayout::eColorAttachmentOptimal)
					.setLoadOp(vk::AttachmentLoadOp::eClear)
					.setStoreOp(vk::AttachmentStoreOp::eStore)
					.setClearValue(clearColour)
				)
			);

			cmdBuf.setViewport(0, vk::Viewport(0.0f, 0.0f, windowSizeExtent.width, windowSizeExtent.height));
			cmdBuf.setScissor(0, rect);
			cmdBuf.endRendering();
			cmdBuf.end();
		}

		for (size_t i = 0; i != vkCore.GetSwapchainImageViews().size(); i++)
		{
			const auto& cmdBuf = m_EndRenderCmdBuf[i];
			cmdBuf.begin(vk::CommandBufferBeginInfo());
			cmdBuf.pipelineBarrier2(vk::DependencyInfo()
				.setImageMemoryBarriers(vk::ImageMemoryBarrier2()
					.setSrcStageMask(vk::PipelineStageFlagBits2::eColorAttachmentOutput)
					.setDstStageMask(vk::PipelineStageFlagBits2::eNone)
					.setSrcAccessMask(vk::AccessFlagBits2::eColorAttachmentWrite)
					.setDstAccessMask(vk::AccessFlagBits2::eNone)
					.setOldLayout(vk::ImageLayout::eColorAttachmentOptimal)
					.setNewLayout(vk::ImageLayout::ePresentSrcKHR)
					.setImage(vkCore.GetSwapchainImages()[i])
					.setSubresourceRange(vk::ImageSubresourceRange()
						.setAspectMask(vk::ImageAspectFlagBits::eColor)
						.setBaseMipLevel(0)
						.setLevelCount(1)
						.setBaseArrayLayer(0)
						.setLayerCount(1)
					)
				)
			);
			cmdBuf.end();
		}
	}

	void VulkanContext::DestroySwapchain()
	{
		const auto& vkCore = VulkanCore::GetConstInstance();

		for (const auto& imageView : vkCore.GetSwapchainImageViews())
			vkCore.GetDevice().destroyImageView(imageView);

		vkCore.GetDevice().destroySwapchainKHR(vkCore.GetSwapchain());
	}

	void VulkanContext::RecreateSwapchain()
	{
		auto& vkCore = VulkanCore::GetInstance();
		WaitForIdle();
		DestroySwapchain();
		vkCore.GetSwapchainImages().clear();
		vkCore.GetSwapchainImageViews().clear();
		CreateSwapchain();
		m_RenderPassNeedsRecreating = true;
	}

	void VulkanContext::AddRenderPassData(const std::shared_ptr<RenderPassData>& renderPassData)
	{
		VulkanCore::GetInstance().AddRenderPassData(renderPassData);
	}

	void VulkanContext::Minimize()
	{
		auto& vkCore = VulkanCore::GetInstance();
		WaitForIdle();
		DestroySwapchain();
		vkCore.GetSwapchainImages().clear();
		vkCore.GetSwapchainImageViews().clear();
		m_IsMinimized = true;
	}

	void VulkanContext::Restore()
	{
		if (!m_IsMinimized)
			return;

		m_IsMinimized = false;
		CreateSwapchain();
		WriteCommandBuffers();
		m_RenderPassNeedsRecreating = true;
	}

	void VulkanContext::ImGuiInit()
	{
		const auto& vsCore = VulkanCore::GetConstInstance();

		std::array<vk::DescriptorPoolSize, 1> poolSizes = {
			vk::DescriptorPoolSize()
				.setType(vk::DescriptorType::eCombinedImageSampler)
				.setDescriptorCount(1000)
		};

		vk::DescriptorPoolCreateInfo poolInfo = vk::DescriptorPoolCreateInfo()
			.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
			.setMaxSets(1000)
			.setPoolSizeCount(static_cast<uint32_t>(poolSizes.size()))
			.setPPoolSizes(poolSizes.data());

		VulkanCore::GetInstance().SetImGuiDescriptorPool(vsCore.GetDevice().createDescriptorPool(poolInfo));

		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.ApiVersion = g_VulkanVersion;
		init_info.Instance = vsCore.GetVKInstance();
		init_info.PhysicalDevice = vsCore.GetPhysicalDev();
		init_info.Device = vsCore.GetDevice();
		init_info.QueueFamily = m_QueueFamilyIndices.GraphicsFamily;
		init_info.Queue = vsCore.GetGraphicsQueue();
		init_info.PipelineCache = nullptr;
		init_info.DescriptorPool = vsCore.GetImGuiDescriptorPool();
		init_info.UseDynamicRendering = true;
		init_info.MinAllocationSize = 1024 * 1024;
//		init_info.MinImageCount = static_cast<uint32_t>(VulkanCore::GetConstInstance().GetSwapchainImages().size());
		init_info.MinImageCount = 2;
		init_info.ImageCount = static_cast<uint32_t>(vsCore.GetSwapchainImages().size());
		init_info.Allocator = nullptr;
		init_info.CheckVkResultFn = [](VkResult err)
		{
			if (err != VK_SUCCESS)
				Log<LogLevel::Error>("ImGui Vulkan Error: {}", vk::to_string(static_cast<vk::Result>(err)));
		};

		vk::PipelineRenderingCreateInfo pipelineInfo = vk::PipelineRenderingCreateInfo()
			.setColorAttachmentCount(1)
			.setColorAttachmentFormats(vsCore.GetSwapchainImageFormat())
			.setDepthAttachmentFormat(vk::Format::eUndefined)
			.setStencilAttachmentFormat(vk::Format::eUndefined);

		init_info.PipelineInfoMain.PipelineRenderingCreateInfo = pipelineInfo;

		ImGui_ImplVulkan_Init(&init_info);
	}

	void VulkanContext::ImGuiShutdown()
	{
		ImGui_ImplVulkan_Shutdown();

		VulkanCore::GetInstance().GetDevice().destroyDescriptorPool(VulkanCore::GetConstInstance().GetImGuiDescriptorPool());
	}

	void VulkanContext::ImGuiNewFrame()
	{
		ImGui_ImplVulkan_NewFrame();
	}
}
