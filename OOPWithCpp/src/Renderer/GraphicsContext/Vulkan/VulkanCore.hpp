#include "Log.hpp"

#include <vulkan/vulkan.hpp>
#include <memory>


namespace OWC::Graphics
{
	constexpr auto g_VulkanVersion = VK_MAKE_VERSION(1, 4, 0);

	inline bool IsExtentionAvailable(const std::vector<vk::ExtensionProperties>& extentions, std::string_view requstExtention)
	{
		for (const auto& ext : extentions)
			if (requstExtention == ext.extensionName)
				return true;

		return false;
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

		[[nodiscard]] vk::CommandBuffer GetGraphicsCommandBuffer();
		[[nodiscard]] vk::CommandBuffer GetComputeCommandBuffer();
		[[nodiscard]] vk::CommandBuffer GetTransferCommandBuffer();

		void BeginRenderPass(const vk::CommandBuffer& commandBuf, vk::Pipeline pipeline) const;
		void EndRenderPass(const vk::CommandBuffer& commandBuf) const;

		void SubmitGraphicsCommandBuffer(const vk::CommandBuffer& commandBuf) const;

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
		[[nodiscard]] inline const vk::SwapchainKHR& GetSwapchain() const { return m_Swapchain; }
		[[nodiscard]] inline const vk::Format& GetSwapchainImageFormat() const { return m_SwapchainImageFormat; }
		[[nodiscard]] inline const std::vector<vk::Image>& GetSwapchainImages() const { return m_SwapchainImages; }
		[[nodiscard]] inline const std::vector<vk::ImageView>& GetSwapchainImageViews() const { return m_SwapchainImageViews; }
		[[nodiscard]] inline const std::vector<vk::Framebuffer>& GetSwapchainFramebuffers() const { return m_SwapchainFramebuffers; }
		[[nodiscard]] inline const vk::RenderPass& GetRenderPass() const { return m_RenderPass; }
		[[nodiscard]] inline size_t GetCurrentFrameIndex() const { return m_CurrentFrameIndex; }


		[[nodiscard]] inline std::vector<vk::Image>& GetSwapchainImages() { return m_SwapchainImages; }
		[[nodiscard]] inline std::vector<vk::ImageView>& GetSwapchainImageViews() { return m_SwapchainImageViews; }
		[[nodiscard]] inline std::vector<vk::Framebuffer>& GetSwapchainFramebuffers() { return m_SwapchainFramebuffers; }
		[[nodiscard]] inline size_t& GetCurrentFrameIndex() { return m_CurrentFrameIndex; }
		[[nodiscard]] inline const vk::Semaphore& GetImageAvailableSemaphore() const { return m_ImageAvailableSemaphore; }
		[[nodiscard]] inline const vk::Fence& GetInFlightFence() const { return m_InFlightFence; }

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
		inline void SetSwapchainImageFormat(const vk::Format& format) { m_SwapchainImageFormat = format; }
		inline void SetSwapchain(const vk::SwapchainKHR& swapchain) { m_Swapchain = swapchain; }
		inline void SetSwapchainImages(const std::vector<vk::Image>& swapchainImages) { m_SwapchainImages = swapchainImages; }
		inline void SetSwapchainImageViews(const std::vector<vk::ImageView>& swapchainImageViews) { m_SwapchainImageViews = swapchainImageViews; }
		inline void SetSwapchainFramebuffers(const std::vector<vk::Framebuffer>& swapchainFramebuffers) { m_SwapchainFramebuffers = swapchainFramebuffers; }
		inline void SetRenderPass(const vk::RenderPass& renderPass) { m_RenderPass = renderPass; }

		inline void IncrementCurrentFrameIndex()
		{
			if (m_ImageAvailableSemaphore)
				m_Device.destroySemaphore(m_ImageAvailableSemaphore);

			m_ImageAvailableSemaphore = m_Device.createSemaphore(vk::SemaphoreCreateInfo());
			m_CurrentFrameIndex = m_Device.acquireNextImage2KHR(
				vk::AcquireNextImageInfoKHR()
				.setSwapchain(m_Swapchain)
				.setSemaphore(m_ImageAvailableSemaphore)
				.setTimeout(16'666)
				.setDeviceMask(1)
			).value;
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
		vk::SwapchainKHR m_Swapchain = vk::SwapchainKHR();
		vk::Format m_SwapchainImageFormat = vk::Format::eUndefined;
		std::vector<vk::Image> m_SwapchainImages = {};
		std::vector<vk::ImageView> m_SwapchainImageViews = {};
		std::vector<vk::Framebuffer> m_SwapchainFramebuffers = {};
		vk::RenderPass m_RenderPass = vk::RenderPass();

		vk::Semaphore m_ImageAvailableSemaphore = vk::Semaphore();
		mutable vk::Fence m_InFlightFence = vk::Fence();
		size_t m_CurrentFrameIndex = 0;

		static std::unique_ptr<VulkanCore> s_Instance;
	};
}
