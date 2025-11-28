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

		[[nodiscard]] inline const vk::Instance& GetVKInstance() const { return m_Instance; }
		[[nodiscard]] inline const vk::SurfaceKHR& GetSurface() const { return m_Surface; }
		[[nodiscard]] inline const vk::PhysicalDevice& GetPhysicalDev() const { return m_PhysicalDevice; }
		[[nodiscard]] inline const vk::Device& GetDevice() const { return m_Device; }
		[[nodiscard]] inline const vk::Queue& GetPresentQueue() const { return m_PresentQueue; }
		[[nodiscard]] inline const vk::Queue& GetGraphicsQueue() const { return m_GraphicsQueue; }
		[[nodiscard]] inline const vk::Queue& GetComputeQueue() const { return m_ComputeQueue; }
		[[nodiscard]] inline const vk::Queue& GetTransferQueue() const { return m_TransferQueue; }
		[[nodiscard]] inline const vk::SwapchainKHR& GetSwapchain() const { return m_Swapchain; }
		[[nodiscard]] inline const std::vector<vk::Image>& GetSwapchainImages() const { return m_SwapchainImages; }
		[[nodiscard]] inline const std::vector<vk::ImageView>& GetSwapchainImageViews() const { return m_SwapchainImageViews; }

		[[nodiscard]] inline std::vector<vk::Image>& GetSwapchainImages() { return m_SwapchainImages; }
		[[nodiscard]] inline std::vector<vk::ImageView>& GetSwapchainImageViews() { return m_SwapchainImageViews; }

		inline void SetInstance(const vk::Instance& instance) { m_Instance = instance; }
		inline void SetSurface(const vk::SurfaceKHR& surface) { m_Surface = surface; }
		inline void SetPhysicalDevice(const vk::PhysicalDevice& physicalDevice) { m_PhysicalDevice = physicalDevice; }
		inline void SetDevice(const vk::Device& device) { m_Device = device; }
		inline void SetPresentQueue(const vk::Queue& presentQueue) { m_PresentQueue = presentQueue; }
		inline void SetGraphicsQueue(const vk::Queue& graphicsQueue) { m_GraphicsQueue = graphicsQueue; }
		inline void SetComputeQueue(const vk::Queue& computeQueue) { m_ComputeQueue = computeQueue; }
		inline void SetTransferQueue(const vk::Queue& transferQueue) { m_TransferQueue = transferQueue; }
		inline void SetSwapchain(const vk::SwapchainKHR& swapchain) { m_Swapchain = swapchain; }
		inline void SetSwapchainImages(const std::vector<vk::Image>& swapchainImages) { m_SwapchainImages = swapchainImages; }
		inline void SetSwapchainImageViews(const std::vector<vk::ImageView>& swapchainImageViews) { m_SwapchainImageViews = swapchainImageViews; }

	private:
		vk::Instance m_Instance = vk::Instance();
		vk::SurfaceKHR m_Surface = vk::SurfaceKHR();
		vk::PhysicalDevice m_PhysicalDevice = vk::PhysicalDevice();
		vk::Device m_Device = vk::Device();
		vk::Queue m_PresentQueue = vk::Queue();
		vk::Queue m_GraphicsQueue = vk::Queue();
		vk::Queue m_ComputeQueue = vk::Queue();
		vk::Queue m_TransferQueue = vk::Queue();
		vk::SwapchainKHR m_Swapchain = vk::SwapchainKHR();
		std::vector<vk::Image> m_SwapchainImages = {};
		std::vector<vk::ImageView> m_SwapchainImageViews = {};

		static std::unique_ptr<VulkanCore> s_Instance;
	};
}
