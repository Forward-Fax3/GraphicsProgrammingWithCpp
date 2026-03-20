#pragma once
#include "Log.hpp"
#include "Renderer.hpp"
#include "VulkanRenderPass.hpp"

#include <vulkan/vulkan.hpp>
#define VMA_VULKAN_VERSION 1004000
#include <list>

#include "vk_mem_alloc.hpp"

#include <mutex>
#include <memory>
#include <map>
#include <ranges>


namespace OWC::Graphics
{
	constexpr auto g_VulkanVersion = VK_MAKE_VERSION(1, 4, 0);

	[[nodiscard]] OWC_FORCE_INLINE bool IsExtentionAvailable(const std::vector<vk::ExtensionProperties>& extentions, std::string_view requstExtention)
	{
		auto it = std::ranges::find_if(extentions, [&requstExtention](const vk::ExtensionProperties& ext) {
			return requstExtention == ext.extensionName;
			});

		return it != extentions.end();
	}

	[[nodiscard]] OWC_FORCE_INLINE std::pair<bool, std::string_view> IsExtentionAvailable(const std::vector<vk::ExtensionProperties>& extentions, std::span<const char*> requstLayer)
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

		static void Init();

		static void Shutdown() { s_Instance.reset(); }

		void AddRenderPassData(const std::shared_ptr<RenderPassData>& data);
		void ResetRenderPassDatas();

		[[nodiscard]] std::vector<vk::CommandBuffer> GetGraphicsCommandBuffer() const;
		[[nodiscard]] std::vector<vk::CommandBuffer> GetComputeCommandBuffer() const;
		[[nodiscard]] std::vector<vk::CommandBuffer> GetTransferCommandBuffer() const;
		[[nodiscard]] std::vector<vk::CommandBuffer> GetDynamicGraphicsCommandBuffer() const;
		[[nodiscard]] std::vector<vk::CommandBuffer> GetDynamicComputeCommandBuffer() const;
		[[nodiscard]] std::vector<vk::CommandBuffer> GetDynamicTransferCommandBuffer() const;
		[[nodiscard]] vk::CommandBuffer GetSingleTimeGraphicsCommandBuffer() const;
		[[nodiscard]] vk::CommandBuffer GetSingleTimeComputeCommandBuffer() const;
		[[nodiscard]] vk::CommandBuffer GetSingleTimeTransferCommandBuffer() const;

		[[nodiscard]] std::vector<vk::Semaphore> GetSemaphoresFromNames(std::span<std::string_view> semaphoreNames);
		[[nodiscard]] vk::Semaphore GetSingleSemaphore() const;

		[[nodiscard]] u32 FindMemoryType(vk::DeviceSize size, vk::MemoryPropertyFlags properties) const;

		[[nodiscard]] OWC_FORCE_INLINE const vk::Instance& GetVKInstance() const { return m_Instance; }
		[[nodiscard]] OWC_FORCE_INLINE const vk::SurfaceKHR& GetSurface() const { return m_Surface; }
		[[nodiscard]] OWC_FORCE_INLINE const vk::PhysicalDevice& GetPhysicalDev() const { return m_PhysicalDevice; }
		[[nodiscard]] OWC_FORCE_INLINE const vk::Device& GetDevice() const { return m_Device; }
		[[nodiscard]] OWC_FORCE_INLINE const vk::Queue& GetPresentQueue() const { return m_PresentQueue; }
		[[nodiscard]] OWC_FORCE_INLINE const vk::Queue& GetGraphicsQueue() const { return m_GraphicsQueue; }
		[[nodiscard]] OWC_FORCE_INLINE const vk::Queue& GetComputeQueue() const { return m_ComputeQueue; }
		[[nodiscard]] OWC_FORCE_INLINE const vk::Queue& GetTransferQueue() const { return m_TransferQueue; }
		[[nodiscard]] OWC_FORCE_INLINE const vk::CommandPool& GetGraphicsCommandPool() const { return m_GraphicsCommandPool; }
		[[nodiscard]] OWC_FORCE_INLINE const vk::CommandPool& GetComputeCommandPool() const { return m_ComputeCommandPool; }
		[[nodiscard]] OWC_FORCE_INLINE const vk::CommandPool& GetTransferCommandPool() const { return m_TransferCommandPool; }
		[[nodiscard]] OWC_FORCE_INLINE const vk::CommandPool& GetDynamicGraphicsCommandPool() const { return m_DynamicGraphicsCommandPool; }
		[[nodiscard]] OWC_FORCE_INLINE const vk::CommandPool& GetDynamicComputeCommandPool() const { return m_DynamicComputeCommandPool; }
		[[nodiscard]] OWC_FORCE_INLINE const vk::CommandPool& GetDynamicTransferCommandPool() const { return m_DynamicTransferCommandPool; }
		[[nodiscard]] OWC_FORCE_INLINE const vk::SwapchainKHR& GetSwapchain() const { return m_Swapchain; }
		[[nodiscard]] OWC_FORCE_INLINE const vk::Format& GetSwapchainImageFormat() const { return m_SwapchainImageFormat; }
		[[nodiscard]] OWC_FORCE_INLINE const vk::DescriptorPool& GetImGuiDescriptorPool() const { return m_ImGuiDescriptorPool; }
		[[nodiscard]] OWC_FORCE_INLINE const std::vector<vk::Image>& GetSwapchainImages() const { return m_SwapchainImages; }
		[[nodiscard]] OWC_FORCE_INLINE const std::vector<vk::ImageView>& GetSwapchainImageViews() const { return m_SwapchainImageViews; }
		[[nodiscard]] OWC_FORCE_INLINE const vma::Allocator& GetVulkanMemoryAllocator() const { return m_Allocator; }
		[[nodiscard]] OWC_FORCE_INLINE uSize GetCurrentFrameIndex() const { return m_CurrentFrameIndex; }
		[[nodiscard]] OWC_FORCE_INLINE uSize GetNumberOfFramesInFlight() const { return m_SwapchainImageViews.size(); }

		[[nodiscard]] OWC_FORCE_INLINE u32 GetGraphicsQueueIndex() const { return m_GraphicsQueueFamilyIndex; }
		[[nodiscard]] OWC_FORCE_INLINE u32 GetComputeQueueIndex() const { return m_ComputeQueueFamilyIndex; }
		[[nodiscard]] OWC_FORCE_INLINE u32 GetTransferQueueIndex() const { return m_TransferQueueFamilyIndex; }
		[[nodiscard]] OWC_FORCE_INLINE u32 GetPresentQueueIndex() const { return m_PresentQueueFamilyIndex; }

		[[nodiscard]] OWC_FORCE_INLINE std::vector<vk::Image>& GetSwapchainImages() { return m_SwapchainImages; }
		[[nodiscard]] OWC_FORCE_INLINE std::vector<vk::ImageView>& GetSwapchainImageViews() { return m_SwapchainImageViews; }
		[[nodiscard]] OWC_FORCE_INLINE std::vector<std::list<std::function<void()>>>& GetEndOfFrameCleanUp() { return m_EndOfFrameCleanUp; }
		[[nodiscard]] OWC_FORCE_INLINE std::pair<std::reference_wrapper<std::vector<std::shared_ptr<VulkanRenderPass>>>, std::unique_lock<std::mutex>> GetRenderPassDatas() { return { std::ref(m_RenderPassDatas), std::unique_lock(m_RenderPassesMutex) }; }

		OWC_FORCE_INLINE void SetInstance(const vk::Instance& instance) { m_Instance = instance; }
		OWC_FORCE_INLINE void SetSurface(const vk::SurfaceKHR& surface) { m_Surface = surface; }
		OWC_FORCE_INLINE void SetPhysicalDevice(const vk::PhysicalDevice& physicalDevice) { m_PhysicalDevice = physicalDevice; }
		OWC_FORCE_INLINE void SetDevice(const vk::Device& device) { m_Device = device; }
		OWC_FORCE_INLINE void SetPresentQueue(const vk::Queue& presentQueue) { m_PresentQueue = presentQueue; }
		OWC_FORCE_INLINE void SetGraphicsQueue(const vk::Queue& graphicsQueue) { m_GraphicsQueue = graphicsQueue; }
		OWC_FORCE_INLINE void SetComputeQueue(const vk::Queue& computeQueue) { m_ComputeQueue = computeQueue; }
		OWC_FORCE_INLINE void SetTransferQueue(const vk::Queue& transferQueue) { m_TransferQueue = transferQueue; }
		OWC_FORCE_INLINE void SetGraphicsCommandPool(const vk::CommandPool& commandPool) { m_GraphicsCommandPool = commandPool; }
		OWC_FORCE_INLINE void SetComputeCommandPool(const vk::CommandPool& commandPool) { m_ComputeCommandPool = commandPool; }
		OWC_FORCE_INLINE void SetTransferCommandPool(const vk::CommandPool& commandPool) { m_TransferCommandPool = commandPool; }
		OWC_FORCE_INLINE void SetDynamicGraphicsCommandPool(const vk::CommandPool& commandPool) { m_DynamicGraphicsCommandPool = commandPool; }
		OWC_FORCE_INLINE void SetDynamicComputeCommandPool(const vk::CommandPool& commandPool) { m_DynamicComputeCommandPool = commandPool; }
		OWC_FORCE_INLINE void SetDynamicTransferCommandPool(const vk::CommandPool& commandPool) { m_DynamicTransferCommandPool = commandPool; }
		OWC_FORCE_INLINE void SetSwapchainImageFormat(const vk::Format& format) { m_SwapchainImageFormat = format; }
		OWC_FORCE_INLINE void SetImGuiDescriptorPool(const vk::DescriptorPool& pool) { m_ImGuiDescriptorPool = pool;  }
		OWC_FORCE_INLINE void SetSwapchain(const vk::SwapchainKHR& swapchain) { m_Swapchain = swapchain; }
		OWC_FORCE_INLINE void SetSwapchainImages(const std::vector<vk::Image>& swapchainImages) { m_SwapchainImages = swapchainImages; }
		OWC_FORCE_INLINE void SetSwapchainImageViews(const std::vector<vk::ImageView>& swapchainImageViews) { m_SwapchainImageViews = swapchainImageViews; }
		OWC_FORCE_INLINE void SetCurrentFrameIndex(uSize newIndex) { m_CurrentFrameIndex = newIndex; }
		OWC_FORCE_INLINE void SetVulkanMemoryAllocator(const vma::Allocator& allocator) { m_Allocator = allocator; }
		OWC_FORCE_INLINE void AddVulkanEndOfFrameCleanUpFunction(std::function<void()> func) { m_EndOfFrameCleanUp[m_CurrentFrameIndex].emplace_back(std::move(func)); }

		OWC_FORCE_INLINE void SetQueueFamilyIndexes(const u32 graphicsIndex, const u32 computeIndex, const u32 transferIndex, const u32 presentIndex)
		{
			m_GraphicsQueueFamilyIndex = graphicsIndex;
			m_ComputeQueueFamilyIndex = computeIndex;
			m_TransferQueueFamilyIndex = transferIndex;
			m_PresentQueueFamilyIndex = presentIndex;
		}

		OWC_FORCE_INLINE void SetupSemaphores()
		{
			m_Semaphores.reserve(m_SwapchainImageViews.size());
			for (uSize i = 0; i < m_SwapchainImageViews.size(); i++)
				m_Semaphores.emplace_back();
		}

		OWC_FORCE_INLINE void DestroySemaphores()
		{
			for (auto& semaphores : m_Semaphores)
			{
				for (auto& semaphore : std::views::values(semaphores))
					if (semaphore)
					{
						m_Device.destroySemaphore(semaphore);
						semaphore = vk::Semaphore();
					}

				semaphores.clear();
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
		std::vector<vk::Image> m_SwapchainImages{};
		std::vector<vk::ImageView> m_SwapchainImageViews{};

		std::vector<std::list<std::function<void()>>> m_EndOfFrameCleanUp{};

		vma::Allocator m_Allocator = vma::Allocator();

		std::vector<std::map<std::string, vk::Semaphore>> m_Semaphores{};
		uSize m_CurrentFrameIndex = 0;

		std::vector<std::shared_ptr<VulkanRenderPass>> m_RenderPassDatas{};

		std::mutex m_RenderPassesMutex;

		u32 m_GraphicsQueueFamilyIndex = 0;
		u32 m_ComputeQueueFamilyIndex = 0;
		u32 m_TransferQueueFamilyIndex = 0;
		u32 m_PresentQueueFamilyIndex = 0;

		static std::unique_ptr<VulkanCore> s_Instance;
	};
}
