#include <limits>
#include <ranges>
#include <mutex>
#include <map>

#include "Core.hpp"

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_to_string.hpp>
#include <SDL3/SDL_vulkan.h>
#include <backends/imgui_impl_vulkan.h>

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
	static OWC::i32 lastMessageID = std::numeric_limits<OWC::i32>::max();
	static vk::DebugUtilsMessageSeverityFlagBitsEXT lastMessageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT();
	static bool firstMessage = true;

	if (pCallbackData == nullptr)
	{
		std::lock_guard lock(s_Mutex);

		if (s_MessagesLogged.empty())
			return vk::False;

		PrintVulkanDebugMessages(lastMessageSeverity, s_MessagesLogged);
		s_MessagesLogged.clear();
		lastMessageID = std::numeric_limits<OWC::i32>::max();
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
		for (OWC::u32 i = 0; i < pCallbackData->queueLabelCount; i++)
		{
			str += std::format("\n\t\t labelName = <{}>",
				pCallbackData->pQueueLabels[i].pLabelName ? pCallbackData->pQueueLabels[i].pLabelName : "NULL");
		}
	}
	if (pCallbackData->cmdBufLabelCount > 0)
	{
		str += "\n\t CommandBuffer Labels:";
		for (OWC::u32 i = 0; i < pCallbackData->cmdBufLabelCount; i++)
		{
			str += std::format("\n\t\t labelName = <{}>",
				pCallbackData->pCmdBufLabels[i].pLabelName ? pCallbackData->pCmdBufLabels[i].pLabelName : "NULL");
		}
	}
//	if (pCallbackData->objectCount > 0 /* && !std::string("NULL").compare(pCallbackData->pObjects[0].pObjectName) */)
//	{
//		str += "\n\t Objects:";
//		for (u32 i = 0; i < pCallbackData->objectCount; i++)
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
			lastMessageID = std::numeric_limits<OWC::i32>::max();
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
	std::array g_DeviceExtensions = {
#if defined(_WIN32) || defined(_WIN64)
		vk::EXTPageableDeviceLocalMemoryExtensionName,
		vk::EXTMemoryPriorityExtensionName,
#endif
		vk::KHRSwapchainExtensionName,
		vk::KHRAccelerationStructureExtensionName,
		vk::KHRRayTracingPipelineExtensionName,
		vk::KHRBufferDeviceAddressExtensionName
	};

	VulkanContext::VulkanContext(SDL_Window& windowHandle, const WindowProperties& properties)
		 : m_WindowProperties(properties)
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
			std::array<std::string_view, 1> semaphoreNames = { "RenderFinished" };
			auto semaphores = vkCore.GetSemaphoresFromNames(semaphoreNames);

			vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eBottomOfPipe);
			const vk::SubmitInfo submitInfo = vk::SubmitInfo()
				.setSignalSemaphores(semaphores[0])
				.setCommandBuffers(m_EndRenderCmdBuf[vkCore.GetCurrentFrameIndex()])
				.setWaitDstStageMask(waitDestinationStageMask)
				.setWaitSemaphores(VK_NULL_HANDLE);

			vkCore.GetGraphicsQueue().submit(submitInfo, VK_NULL_HANDLE);

			auto indices = static_cast<u32>(vkCore.GetCurrentFrameIndex());
			const auto result = VulkanCore::GetConstInstance().GetPresentQueue().presentKHR(
				vk::PresentInfoKHR()
				.setSwapchains(vkCore.GetSwapchain())
				.setSwapchainCount(1)
				.setImageIndices(indices)
				.setWaitSemaphores(semaphores[0])
			);

			(void)result;

//			uSize retryCount = 0;
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

		std::array<std::string_view, 1> imageAcquiredName = { "ImageAcquired" };
		auto imageAcquired = vkCore.GetSemaphoresFromNames(imageAcquiredName)[0];

		auto result = vkCore.GetDevice().acquireNextImage2KHR(vk::AcquireNextImageInfoKHR()
			.setSwapchain(vkCore.GetSwapchain())
			.setSemaphore(imageAcquired)
//			.setTimeout(16'666)
			.setTimeout(std::numeric_limits<u32>::max())
			.setDeviceMask(1)
		);

		if (result.result == vk::Result::eErrorOutOfDateKHR || m_RenderPassNeedsRecreating)
		{
			RecreateSwapchain();
			result = vkCore.GetDevice().acquireNextImage2KHR(vk::AcquireNextImageInfoKHR()
				.setSwapchain(vkCore.GetSwapchain())
				.setSemaphore(imageAcquired)
//				.setTimeout(16'666)
				.setTimeout(std::numeric_limits<u32>::max())
				.setDeviceMask(1)
			);
			RewriteCommandBuffers();

			if (result.result == vk::Result::eErrorOutOfDateKHR)
				Log<LogLevel::Critical>("Failed to acquire next image from swapchain: {}", vk::to_string(result.result));
		}
		else if (result.result != vk::Result::eSuccess && result.result != vk::Result::eSuboptimalKHR)
			Log<LogLevel::Critical>("Failed to acquire next image from swapchain: {}", vk::to_string(result.result));

		// this should never happen since acquireNextImage2KHR should return an error if the index is out of bounds, but just in case
		if (result.value >= vkCore.GetSwapchainImageViews().size())
			Log<LogLevel::Critical>("Acquired image index {} is out of bounds for swapchain image views size {}", result.value, vkCore.GetSwapchainImageViews().size());

		vkCore.SetCurrentFrameIndex(result.value);

//		uSize retryCount = 0;
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
		const vk::SubmitInfo submitInfo = vk::SubmitInfo()
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
		std::vector<const char*> extensions;
		{
			u32 numberOfSDLExtensions = 0;
			const auto extensionsTemp = SDL_Vulkan_GetInstanceExtensions(&numberOfSDLExtensions);

			if constexpr (!IsDistributionMode()) // +3 for debug utils and get physical device properties 2 and surface extensions
				extensions.reserve(static_cast<uSize>(numberOfSDLExtensions) + 3);
			else // +2 for get physical device properties 2 and surface extensions
				extensions.reserve(static_cast<uSize>(numberOfSDLExtensions) + 2);

			for (uSize i = 0; i < numberOfSDLExtensions; i++)
				extensions.emplace_back(extensionsTemp[i]);
		}

		const auto instanceExtensionProperties = vk::enumerateInstanceExtensionProperties();

		extensions.emplace_back(vk::KHRGetSurfaceCapabilities2ExtensionName);

		vk::InstanceCreateInfo createInfo;
		
		std::vector<const char*> validationLayers;
		if constexpr (!IsDistributionMode())
		{
			extensions.emplace_back(vk::EXTDebugUtilsExtensionName);

			validationLayers.push_back("VK_LAYER_KHRONOS_validation");
			createInfo.setEnabledLayerCount(static_cast<u32>(validationLayers.size()));
			createInfo.setPpEnabledLayerNames(validationLayers.data());
		}

		const auto [extensionsAvailable, missingExtension] = IsExtentionAvailable(instanceExtensionProperties, extensions);

		if (!extensionsAvailable)
			Log<LogLevel::Critical>("Vulkan instance is missing required extension: {}", missingExtension);

		constexpr vk::ApplicationInfo appInfo = vk::ApplicationInfo()
			.setPApplicationName("OOPWithCpp Application")
			.setApplicationVersion(VK_MAKE_VERSION(0, 0, 1))
			.setPEngineName("OOPWithCpp Engine")
			.setEngineVersion(VK_MAKE_VERSION(0, 0, 1))
			.setApiVersion(g_VulkanVersion);

		createInfo
			.setEnabledExtensionCount(static_cast<u32>(extensions.size()))
			.setPpEnabledExtensionNames(extensions.data())
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

		u32 highestScore = 0;
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

	std::pair<bool, u32> VulkanContext::IsPhysicalDeviceSuitable(const vk::PhysicalDevice& device)
	{
		u32 score = 0;
		auto deviceProperties = device.getProperties2();
		auto supportedExtensions = device.enumerateDeviceExtensionProperties();

		if (deviceProperties.properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
			score += 1000;

		score += deviceProperties.properties.limits.maxImageDimension2D;

		const auto [extensionsAvailable, missingExtension] = IsExtentionAvailable(supportedExtensions, g_DeviceExtensions);
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
		constexpr u32 indexMax = std::numeric_limits<u32>::max();

		for (u32 i = 0; i < static_cast<u32>(queueFamilies.size()); i++) // try to find all queue families in one loop and have different queue families if possible
		{
			OWC::Log<OWC::LogLevel::Trace>(
				"Queue Family {}: {} queues, flags: {}, present supported: {}",
				i,
				queueFamilies[i].queueCount,
				vk::to_string(queueFamilies[i].queueFlags),
				static_cast<bool>(VulkanCore::GetConstInstance().GetPhysicalDev().getSurfaceSupportKHR(
					i,
					VulkanCore::GetConstInstance().GetSurface()
				))
			);

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

	void VulkanContext::CheckQueueFamilyValidity(const std::vector<vk::QueueFamilyProperties>& queueFamilies)
	{
		constexpr u32 indexMax = std::numeric_limits<u32>::max();

		std::map<u32, u32> numberOfUsesPerQueueFamily;
		for (const auto& index : { m_QueueFamilyIndices.PresentFamily, m_QueueFamilyIndices.GraphicsFamily, m_QueueFamilyIndices.ComputeFamily, m_QueueFamilyIndices.TransferFamily })
		{
			if (index == indexMax)
				continue;

			if (!numberOfUsesPerQueueFamily.contains(index))
				numberOfUsesPerQueueFamily[index] = 1;
			// no need for else as there will only be one per family index at this point
		}

		if (m_QueueFamilyIndices.PresentFamily == indexMax)
			Log<LogLevel::Critical>("Failed to find a valid present queue family index");
		else if (m_QueueFamilyIndices.GraphicsFamily == indexMax)
			Log<LogLevel::Critical>("Failed to find a valid graphics queue family index");
		else if (m_QueueFamilyIndices.ComputeFamily == indexMax) // check if compute queue family is found, otherwise find any even if it shares with others
		{
			Log<LogLevel::Warn>("Failed to find a unique compute queue family index, any compute queue will be searched for now");

			for (u32 i = 0; i < static_cast<u32>(queueFamilies.size()); i++)
				if ((queueFamilies[i].queueFlags & vk::QueueFlagBits::eCompute) == vk::QueueFlagBits::eCompute &&
					queueFamilies[i].queueCount > numberOfUsesPerQueueFamily[i]
					)
				{
					m_QueueFamilyIndices.ComputeFamily = i;
					numberOfUsesPerQueueFamily[i]++;
					break;
				}
		}

		if (m_QueueFamilyIndices.ComputeFamily == indexMax)
			Log<LogLevel::Critical>("Failed to find a valid compute queue family index");
		else if (m_QueueFamilyIndices.TransferFamily == indexMax) // check if transfer queue family is found, otherwise find any even if it shares with others
		{
			Log<LogLevel::Trace>("Failed to find a unique transfer queue family index, any transfer queue will be searched for now");

			for (u32 i = 0; i < static_cast<u32>(queueFamilies.size()); i++)
				if ((queueFamilies[i].queueFlags & vk::QueueFlagBits::eTransfer) == vk::QueueFlagBits::eTransfer &&
					queueFamilies[i].queueCount > numberOfUsesPerQueueFamily[i]
					)
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
		std::map<u32, u32> queueFamilyUsageCount;

		auto l_getQueue = [&](const u32 familyIndex) -> vk::Queue {
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
		std::map<u32, std::pair<u32, std::vector<f32>>> uniqueQueueFamiliesMap;

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

		vk::PhysicalDeviceMemoryPriorityFeaturesEXT memoryPriorityFeature = vk::PhysicalDeviceMemoryPriorityFeaturesEXT()
			.setPNext(nullptr)
			.setMemoryPriority(vk::True);

		vk::PhysicalDevicePageableDeviceLocalMemoryFeaturesEXT pageableDeviceLocalMemoryFeature = vk::PhysicalDevicePageableDeviceLocalMemoryFeaturesEXT()
			.setPNext(&memoryPriorityFeature)
			.setPageableDeviceLocalMemory(vk::True);

		vk::PhysicalDeviceShaderObjectFeaturesEXT shaderObjectFeature = vk::PhysicalDeviceShaderObjectFeaturesEXT()
			.setPNext(&pageableDeviceLocalMemoryFeature)
			.setShaderObject(vk::True);

		vk::PhysicalDeviceShaderDrawParameterFeatures shaderDrawParametersFeature = vk::PhysicalDeviceShaderDrawParameterFeatures()
			.setPNext(&shaderObjectFeature)
			.setShaderDrawParameters(vk::True);

		vk::PhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeature = vk::PhysicalDeviceAccelerationStructureFeaturesKHR()
			.setPNext(&shaderDrawParametersFeature)
			.setAccelerationStructure(vk::True);

		vk::PhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineFeature = vk::PhysicalDeviceRayTracingPipelineFeaturesKHR()
			.setPNext(&accelerationStructureFeature)
			.setRayTracingPipeline(vk::True);

		vk::PhysicalDeviceSwapchainMaintenance1FeaturesKHR swapchainMaintenance1Feature = vk::PhysicalDeviceSwapchainMaintenance1FeaturesKHR()
			.setPNext(&rayTracingPipelineFeature)
			.setSwapchainMaintenance1(vk::True);

		vk::PhysicalDeviceDescriptorIndexingFeatures descriptorIndexingFeature = vk::PhysicalDeviceDescriptorIndexingFeatures()
			.setPNext(&swapchainMaintenance1Feature)
			.setRuntimeDescriptorArray(vk::True)
			.setDescriptorBindingPartiallyBound(vk::True)
			.setDescriptorBindingVariableDescriptorCount(vk::True)
			.setShaderSampledImageArrayNonUniformIndexing(vk::True)
			.setShaderStorageBufferArrayNonUniformIndexing(vk::True)
			.setShaderUniformBufferArrayNonUniformIndexing(vk::True);

		vk::PhysicalDeviceSynchronization2Features synchronization2Feature = vk::PhysicalDeviceSynchronization2Features()
			.setPNext(descriptorIndexingFeature)
			.setSynchronization2(vk::True);

		vk::PhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeature = vk::PhysicalDeviceDynamicRenderingFeatures()
			.setPNext(&synchronization2Feature)
			.setDynamicRendering(vk::True);

		vk::PhysicalDeviceFeatures2 enabledFeatures = vk::PhysicalDeviceFeatures2()
			.setPNext(&dynamicRenderingFeature)
			.setFeatures(vk::PhysicalDeviceFeatures()
				.setSamplerAnisotropy(vk::True)
				.setSampleRateShading(vk::True)
				.setFillModeNonSolid(vk::True)
				.setWideLines(vk::True)
				.setPipelineStatisticsQuery(vk::True));

		VulkanCore::GetInstance().SetDevice(
			VulkanCore::GetConstInstance().GetPhysicalDev().createDevice(
				vk::DeviceCreateInfo()
				.setQueueCreateInfos(deviceQueueCreateInfos)
				.setPEnabledExtensionNames(g_DeviceExtensions)
				.setPNext(&enabledFeatures)
			)
		);

		m_QueueFamilyIndices.UniqueIndices.reserve(uniqueQueueFamiliesMap.size());
		for (const auto& familyIndex : std::views::keys(uniqueQueueFamiliesMap))
			m_QueueFamilyIndices.UniqueIndices.emplace_back(familyIndex);
	}

	void VulkanContext::CreateSwapchain() const
	{
		if (m_IsMinimized)
			return;

		auto& vkCore = VulkanCore::GetConstInstance();

		std::vector<vk::SurfaceFormatKHR> surfaceFormats = vkCore.GetPhysicalDev().getSurfaceFormatsKHR(vkCore.GetSurface());
		if (surfaceFormats.empty())
			Log<LogLevel::Critical>("Failed to find any surface formats for the swapchain");

		VulkanCore::GetInstance().SetSwapchainImageFormat((surfaceFormats[0].format == vk::Format::eUndefined) ? vk::Format::eR8G8B8A8Unorm : surfaceFormats[0].format);
		const vk::ColorSpaceKHR colourSpace = surfaceFormats[0].colorSpace;

		Log<LogLevel::Debug>("Vulkan Swapchain Surface Format selected:");
		Log<LogLevel::Debug>(" Format: {}", vk::to_string(vkCore.GetSwapchainImageFormat()));
		Log<LogLevel::Debug>(" Color Space: {}", vk::to_string(colourSpace));

		vk::SurfaceCapabilities2KHR surfaceCapabilities = vkCore.GetPhysicalDev().getSurfaceCapabilities2KHR(vkCore.GetSurface());

		if (surfaceCapabilities.surfaceCapabilities.currentExtent.width == 0 || surfaceCapabilities.surfaceCapabilities.currentExtent.height == 0)
			Log<LogLevel::Critical>("Failed to get valid surface extents for the swapchain");

		// SDL gives the actual window size, so we can use that directly
		vk::Extent2D swapchainExtent{};
		swapchainExtent.width = m_WindowProperties.PixelWidth;
		swapchainExtent.height = m_WindowProperties.PixelHeight;
		Log<LogLevel::Debug>("Vulkan Swapchain Surface Capabilities:");
		Log<LogLevel::Debug>(" Current Extent: {}x{}", swapchainExtent.width, swapchainExtent.height);

		auto presentModeCompatibilities = vkCore.GetPhysicalDev().getSurfacePresentModesKHR(vkCore.GetSurface());

		if (presentModeCompatibilities.empty())
			Log<LogLevel::Critical>("Failed to find any present modes for the swapchain");

		Log<LogLevel::Debug>("Vulkan Swapchain Present Modes supported:");
		for (const auto& presentMode : presentModeCompatibilities)
			Log<LogLevel::Debug>(" {}", vk::to_string(presentMode));

		auto selectedPresentMode = vk::PresentModeKHR::eFifo; // guaranteed to be available
		// prefer mailbox present mode if available
		/*
		for (const auto& presentMode : presentModeCompatibilities)
			if (presentMode == vk::PresentModeKHR::eMailbox)
			{
				selectedPresentMode = vk::PresentModeKHR::eMailbox;
				break;
			}
		*/
		Log<LogLevel::Debug>("Vulkan Swapchain Present Mode selected: {}", vk::to_string(selectedPresentMode));

		const vk::SurfaceTransformFlagBitsKHR preTransform = (surfaceCapabilities.surfaceCapabilities.supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity)
			? vk::SurfaceTransformFlagBitsKHR::eIdentity
			: surfaceCapabilities.surfaceCapabilities.currentTransform;

		vk::SwapchainCreateInfoKHR swapchainCreateInfo = vk::SwapchainCreateInfoKHR()
			.setSurface(vkCore.GetSurface())
			.setMinImageCount(std::max(surfaceCapabilities.surfaceCapabilities.minImageCount, 2u))
			.setImageFormat(vkCore.GetSwapchainImageFormat())
			.setImageColorSpace(colourSpace)
			.setImageExtent(swapchainExtent)
			.setImageArrayLayers(1)
			.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
			.setImageSharingMode(vk::SharingMode::eExclusive)
			.setPreTransform(preTransform)
			.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
			.setPresentMode(selectedPresentMode)
			.setClipped(vk::True);

		if (m_QueueFamilyIndices.UniqueIndices.size() > 1)
			swapchainCreateInfo
				.setImageSharingMode(vk::SharingMode::eConcurrent)
				.setQueueFamilyIndexCount(static_cast<u32>(m_QueueFamilyIndices.UniqueIndices.size()))
				.setPQueueFamilyIndices(m_QueueFamilyIndices.UniqueIndices.data());

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

			Log<LogLevel::Debug>(" Created image view for swapchain image with handle {}", static_cast<void*>(imageViewCreateInfo.image));
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

		constexpr vk::ClearValue clearColour(vk::ClearColorValue(std::array<f32, 4>{0.0, 0.0, 0.0, 1.0}));

		const vk::Rect2D rect(
			vk::Offset2D(0, 0),
			vk::Extent2D(m_WindowProperties.PixelWidth, m_WindowProperties.PixelHeight)
		);

		for (uSize i = 0; i != vkCore.GetSwapchainImageViews().size(); i++)
		{
			Log<LogLevel::Debug>("PixelSize at record time: {}x{}",
					m_WindowProperties.PixelWidth,
					m_WindowProperties.PixelHeight
				);
			const auto& cmdBuf = m_BeginRenderCmdBuf[i];

			std::array<vk::ImageMemoryBarrier2, 1> imageMemoryBarrier = { vk::ImageMemoryBarrier2()
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
					) };

			cmdBuf.begin(vk::CommandBufferBeginInfo());
			cmdBuf.pipelineBarrier2(vk::DependencyInfo()
				.setImageMemoryBarriers(imageMemoryBarrier)
			);

			const std::array<vk::RenderingAttachmentInfo, 1> renderingAttachmentInfo = {
				vk::RenderingAttachmentInfo()
					.setImageView(vkCore.GetSwapchainImageViews()[i])
					.setImageLayout(vk::ImageLayout::eColorAttachmentOptimal)
					.setLoadOp(vk::AttachmentLoadOp::eClear)
					.setStoreOp(vk::AttachmentStoreOp::eStore)
					.setClearValue(clearColour)
			};

			cmdBuf.beginRendering(vk::RenderingInfo()
				.setRenderArea(rect)
				.setLayerCount(1)
				.setColorAttachmentCount(1)
				.setColorAttachments(renderingAttachmentInfo)
			);

			cmdBuf.setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<f32>(m_WindowProperties.PixelWidth), static_cast<f32>(m_WindowProperties.PixelHeight)));
			cmdBuf.setScissor(0, rect);
			cmdBuf.endRendering();
			cmdBuf.end();
		}

		for (uSize i = 0; i != vkCore.GetSwapchainImageViews().size(); i++)
		{
			const auto& cmdBuf = m_EndRenderCmdBuf[i];
			cmdBuf.begin(vk::CommandBufferBeginInfo());

			const std::array<vk::ImageMemoryBarrier2, 1> imageMemoryBarrier = {
				vk::ImageMemoryBarrier2()
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
			};

			cmdBuf.pipelineBarrier2(vk::DependencyInfo()
				.setImageMemoryBarriers(imageMemoryBarrier)
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

	void VulkanContext::RewriteCommandBuffers()
	{
		const auto& vkCore = VulkanCore::GetInstance();
		WaitForIdle();

		vkCore.GetDevice().freeCommandBuffers(vkCore.GetGraphicsCommandPool(), m_BeginRenderCmdBuf);
		vkCore.GetDevice().freeCommandBuffers(vkCore.GetGraphicsCommandPool(), m_EndRenderCmdBuf);

		WriteCommandBuffers();
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
		RewriteCommandBuffers();
		m_RenderPassNeedsRecreating = true;
	}
	
	void VulkanContext::ImGuiInit()
	{
		const auto& vkCore = VulkanCore::GetConstInstance();

		std::array<vk::DescriptorPoolSize, 1> poolSizes = {
			vk::DescriptorPoolSize()
				.setType(vk::DescriptorType::eCombinedImageSampler)
				.setDescriptorCount(1000)
		};

		const auto poolInfo = vk::DescriptorPoolCreateInfo()
			.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
			.setMaxSets(1000)
			.setPoolSizes(poolSizes);

		VulkanCore::GetInstance().SetImGuiDescriptorPool(vkCore.GetDevice().createDescriptorPool(poolInfo));

		ImGui_ImplVulkan_InitInfo initInfo = {};
		initInfo.ApiVersion = g_VulkanVersion;
		initInfo.Instance = vkCore.GetVKInstance();
		initInfo.PhysicalDevice = vkCore.GetPhysicalDev();
		initInfo.Device = vkCore.GetDevice();
		initInfo.QueueFamily = m_QueueFamilyIndices.GraphicsFamily;
		initInfo.Queue = vkCore.GetGraphicsQueue();
		initInfo.PipelineCache = nullptr;
		initInfo.DescriptorPool = vkCore.GetImGuiDescriptorPool();
		initInfo.UseDynamicRendering = true;
		initInfo.MinAllocationSize = 1024 * 1024;
		initInfo.MinImageCount = static_cast<u32>(vkCore.GetSwapchainImages().size());
		initInfo.ImageCount = static_cast<u32>(vkCore.GetSwapchainImages().size());
		initInfo.Allocator = nullptr;
		initInfo.CheckVkResultFn = [](VkResult err)
		{
			if (err != VK_SUCCESS)
				Log<LogLevel::Error>("ImGui Vulkan Error: {}", vk::to_string(static_cast<vk::Result>(err)));
		};

		vk::PipelineRenderingCreateInfo pipelineInfo = vk::PipelineRenderingCreateInfo()
			.setColorAttachmentCount(1)
			.setColorAttachmentFormats(vkCore.GetSwapchainImageFormat())
			.setDepthAttachmentFormat(vk::Format::eUndefined)
			.setStencilAttachmentFormat(vk::Format::eUndefined);

		initInfo.PipelineInfoMain.PipelineRenderingCreateInfo = pipelineInfo;

		ImGui_ImplVulkan_Init(&initInfo);
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
