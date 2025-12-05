#pragma once
#include "Log.hpp"
#include "Renderer.hpp"
#include "VulkanRenderPass.hpp"

#include <vulkan/vulkan.hpp>
#include <mutex>
#include <memory>
#include <map>
#include <ranges>


namespace OWC::Graphics
{
	constexpr auto g_VulkanVersion = VK_MAKE_VERSION(1, 4, 0);

	[[nodiscard]] inline bool IsExtentionAvailable(const std::vector<vk::ExtensionProperties>& extentions, std::string_view requstExtention)
	{
		auto it = std::ranges::find_if(extentions, [&requstExtention](const vk::ExtensionProperties& ext) {
			return requstExtention == ext.extensionName;
			});

		return it != extentions.end();
	}

	[[nodiscard]] inline std::pair<bool, std::string_view> IsExtentionAvailable(const std::vector<vk::ExtensionProperties>& extentions, std::span<const char*> requstLayer)
	{
		bool allAvailable = true;
		std::string_view missingExt;
		for (const std::string_view requstExt : requstLayer)
		{
			auto it = std::ranges::find_if(extentions, [&requstExt](const vk::ExtensionProperties& ext) {
				return ext.extensionName == requstExt;
				}
			);

			if (it == extentions.end())
			{
				allAvailable = false;
				missingExt = requstExt;
				break;
			}
		}

		return { allAvailable, missingExt };
	}

	class VulkanCore
	{
	private:
		class PRIVATE {};

	public:
		VulkanCore() = delete;
		~VulkanCore() = default;

		explicit VulkanCore(PRIVATE) {};

		// Delete copy constructor and assignment operator to enforce singleton pattern
		// Also delete move constructor and move assignment operator
		// to prevent taking ownership of the singleton instance
		VulkanCore(const VulkanCore&) = delete;
		VulkanCore& operator=(const VulkanCore&) = delete;
		VulkanCore(VulkanCore&&) = delete;
		VulkanCore& operator=(VulkanCore&&) = delete;

		static VulkanCore& GetInstance() { return *s_Instance; }
		static const VulkanCore& GetConstInstance() { return *s_Instance; }

		static void Init()
		{
			if (!s_Instance)
				s_Instance = std::make_unique<VulkanCore>(PRIVATE());
			else
			{
				Log<LogLevel::Warn>("VulkanCore instance already exists!");
				return;
			}

			const auto runtimeVersion = vk::enumerateInstanceVersion();
			if (runtimeVersion < g_VulkanVersion)
				Log<LogLevel::Critical>("Vulkan runtime version {}.{}.{} is lower than the required version {}.{}.{}",
					VK_VERSION_MAJOR(runtimeVersion), VK_VERSION_MINOR(runtimeVersion), VK_VERSION_PATCH(runtimeVersion),
					VK_VERSION_MAJOR(g_VulkanVersion), VK_VERSION_MINOR(g_VulkanVersion), VK_VERSION_PATCH(g_VulkanVersion));
			else
				Log<LogLevel::Trace>("Vulkan runtime version {}.{}.{} detected",
					VK_VERSION_MAJOR(runtimeVersion), VK_VERSION_MINOR(runtimeVersion), VK_VERSION_PATCH(runtimeVersion));
		}

		static void Shutdown() { s_Instance.reset(); }

		void AddRenderPassData(const std::shared_ptr<RenderPassData>& data);
		void ResetRenderPassDatas();

		[[nodiscard]] std::vector<vk::CommandBuffer> GetGraphicsCommandBuffer() const;
		[[nodiscard]] std::vector<vk::CommandBuffer> GetComputeCommandBuffer() const;
		[[nodiscard]] std::vector<vk::CommandBuffer> GetTransferCommandBuffer() const;
		[[nodiscard]] std::vector<vk::CommandBuffer> GetDynamicGraphicsCommandBuffer() const;
		[[nodiscard]] std::vector<vk::CommandBuffer> GetDynamicComputeCommandBuffer() const;
		[[nodiscard]] std::vector<vk::CommandBuffer> GetDynamicTransferCommandBuffer() const;

		[[nodiscard]] std::vector<vk::Semaphore> GetSemaphoresFromNames(std::span<std::string_view> semaphoreNames);

		[[nodiscard]] inline const vk::Instance& GetVKInstance() const { return m_Instance; }
		[[nodiscard]] inline const vk::SurfaceKHR& GetSurface() const { return m_Surface; }
		[[nodiscard]] inline const vk::PhysicalDevice& GetPhysicalDev() const { return m_PhysicalDevice; }
		[[nodiscard]] inline const vk::Device& GetDevice() const { return m_Device; }
		[[nodiscard]] inline const vk::Queue& GetPresentQueue() const { return m_PresentQueue; }
		[[nodiscard]] inline const vk::Queue& GetGraphicsQueue() const { return m_GraphicsQueue; }
		[[nodiscard]] inline const vk::Queue& GetComputeQueue() const { return m_ComputeQueue; }
		[[nodiscard]] inline const vk::Queue& GetTransferQueue() const { return m_TransferQueue; }
		[[nodiscard]] inline const vk::CommandPool& GetGraphicsCommandPool() const { return m_GraphicsCommandPool; }
		[[nodiscard]] inline const vk::CommandPool& GetComputeCommandPool() const { return m_ComputeCommandPool; }
		[[nodiscard]] inline const vk::CommandPool& GetTransferCommandPool() const { return m_TransferCommandPool; }
		[[nodiscard]] inline const vk::CommandPool& GetDynamicGraphicsCommandPool() const { return m_DynamicGraphicsCommandPool; }
		[[nodiscard]] inline const vk::CommandPool& GetDynamicComputeCommandPool() const { return m_DynamicComputeCommandPool; }
		[[nodiscard]] inline const vk::CommandPool& GetDynamicTransferCommandPool() const { return m_DynamicTransferCommandPool; }
		[[nodiscard]] inline const vk::SwapchainKHR& GetSwapchain() const { return m_Swapchain; }
		[[nodiscard]] inline const vk::Format& GetSwapchainImageFormat() const { return m_SwapchainImageFormat; }
		[[nodiscard]] inline const vk::DescriptorPool& GetImGuiDescriptorPool() const { return m_ImGuiDescriptorPool; }
		[[nodiscard]] inline const std::vector<vk::Image>& GetSwapchainImages() const { return m_SwapchainImages; }
		[[nodiscard]] inline const std::vector<vk::ImageView>& GetSwapchainImageViews() const { return m_SwapchainImageViews; }
		[[nodiscard]] inline size_t GetCurrentFrameIndex() const { return m_CurrentFrameIndex; }

		[[nodiscard]] inline std::vector<vk::Image>& GetSwapchainImages() { return m_SwapchainImages; }
		[[nodiscard]] inline std::vector<vk::ImageView>& GetSwapchainImageViews() { return m_SwapchainImageViews; }
		[[nodiscard]] inline size_t& GetCurrentFrameIndex() { return m_CurrentFrameIndex; }
		[[nodiscard]] inline std::pair<std::reference_wrapper<std::vector<std::shared_ptr<VulkanRenderPass>>>, std::unique_lock<std::mutex>> GetRenderPassDatas() { return { std::ref(m_RenderPassDatas), std::unique_lock(m_RenderPassesMutex) }; }

		inline void SetInstance(const vk::Instance& instance) { m_Instance = instance; }
		inline void SetSurface(const vk::SurfaceKHR& surface) { m_Surface = surface; }
		inline void SetPhysicalDevice(const vk::PhysicalDevice& physicalDevice) { m_PhysicalDevice = physicalDevice; }
		inline void SetDevice(const vk::Device& device) { m_Device = device; }
		inline void SetPresentQueue(const vk::Queue& presentQueue) { m_PresentQueue = presentQueue; }
		inline void SetGraphicsQueue(const vk::Queue& graphicsQueue) { m_GraphicsQueue = graphicsQueue; }
		inline void SetComputeQueue(const vk::Queue& computeQueue) { m_ComputeQueue = computeQueue; }
		inline void SetTransferQueue(const vk::Queue& transferQueue) { m_TransferQueue = transferQueue; }
		inline void SetGraphicsCommandPool(const vk::CommandPool& commandPool) { m_GraphicsCommandPool = commandPool; }
		inline void SetComputeCommandPool(const vk::CommandPool& commandPool) { m_ComputeCommandPool = commandPool; }
		inline void SetTransferCommandPool(const vk::CommandPool& commandPool) { m_TransferCommandPool = commandPool; }
		inline void SetDynamicGraphicsCommandPool(const vk::CommandPool& commandPool) { m_DynamicGraphicsCommandPool = commandPool; }
		inline void SetDynamicComputeCommandPool(const vk::CommandPool& commandPool) { m_DynamicComputeCommandPool = commandPool; }
		inline void SetDynamicTransferCommandPool(const vk::CommandPool& commandPool) { m_DynamicTransferCommandPool = commandPool; }
		inline void SetSwapchainImageFormat(const vk::Format& format) { m_SwapchainImageFormat = format; }
		inline void SetImGuiDescriptorPool(const vk::DescriptorPool& pool) { m_ImGuiDescriptorPool = pool;  }
		inline void SetSwapchain(const vk::SwapchainKHR& swapchain) { m_Swapchain = swapchain; }
		inline void SetSwapchainImages(const std::vector<vk::Image>& swapchainImages) { m_SwapchainImages = swapchainImages; }
		inline void SetSwapchainImageViews(const std::vector<vk::ImageView>& swapchainImageViews) { m_SwapchainImageViews = swapchainImageViews; }

		inline void SetupSemaphores()
		{
			m_Semaphores.reserve(m_SwapchainImageViews.size());
			for (size_t i = 0; i < m_SwapchainImageViews.size(); i++)
				m_Semaphores.emplace_back(std::map<std::string, vk::Semaphore>());
		}

		[[nodiscard]] inline std::pair<vk::Result, vk::Semaphore> IncrementCurrentFrameIndex()
		{
			std::array<std::string_view, 1> imageAcquiredName = { "ImageAcquired" };
			auto imageAcquired = GetSemaphoresFromNames(imageAcquiredName)[0];

			auto result = m_Device.acquireNextImage2KHR(vk::AcquireNextImageInfoKHR()
				.setSwapchain(m_Swapchain)
				.setSemaphore(imageAcquired)
				.setTimeout(16'666)
				.setDeviceMask(1)
			);

			m_CurrentFrameIndex = result.value;

			return { result.result, imageAcquired };
		}

		inline void DestroySemaphores()
		{
			for (auto& semaphores : m_Semaphores)
				for (auto& semaphore : std::views::values(semaphores))
					if (semaphore)
					{
						m_Device.destroySemaphore(semaphore);
						semaphore = vk::Semaphore();
					}

			m_Semaphores.clear();
		}

	private:
		vk::Instance m_Instance = vk::Instance();
		vk::SurfaceKHR m_Surface = vk::SurfaceKHR();
		vk::PhysicalDevice m_PhysicalDevice = vk::PhysicalDevice();
		vk::Device m_Device = vk::Device();
		vk::Queue m_PresentQueue = vk::Queue();
		vk::Queue m_GraphicsQueue = vk::Queue();
		vk::Queue m_ComputeQueue = vk::Queue();
		vk::Queue m_TransferQueue = vk::Queue();
		vk::CommandPool m_GraphicsCommandPool = vk::CommandPool();
		vk::CommandPool m_ComputeCommandPool = vk::CommandPool();
		vk::CommandPool m_TransferCommandPool = vk::CommandPool();
		vk::CommandPool m_DynamicGraphicsCommandPool = vk::CommandPool();
		vk::CommandPool m_DynamicComputeCommandPool = vk::CommandPool();
		vk::CommandPool m_DynamicTransferCommandPool = vk::CommandPool();
		vk::SwapchainKHR m_Swapchain = vk::SwapchainKHR();
		vk::Format m_SwapchainImageFormat = vk::Format::eUndefined;
		vk::DescriptorPool m_ImGuiDescriptorPool = vk::DescriptorPool();
		std::vector<vk::Image> m_SwapchainImages = {};
		std::vector<vk::ImageView> m_SwapchainImageViews = {};

		std::vector<std::map<std::string, vk::Semaphore>> m_Semaphores = {};
		size_t m_CurrentFrameIndex = 0;

		std::vector<std::shared_ptr<VulkanRenderPass>> m_RenderPassDatas = {};

		std::mutex m_RenderPassesMutex;

		static std::unique_ptr<VulkanCore> s_Instance;
	};
}
