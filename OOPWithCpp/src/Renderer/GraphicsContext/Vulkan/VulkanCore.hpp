#include "core.hpp"

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
		}

		static void Shutdown() { s_Instance.reset(); }

		vk::Instance& GetVKInstance() { return m_Instance; }
		const vk::Instance& GetVKInstance() const { return m_Instance; }
		vk::PhysicalDevice& GetPhysicalDev() { return m_PhysicalDevice; }
		const vk::PhysicalDevice& GetPhysicalDev() const { return m_PhysicalDevice; }
		vk::Device& GetDevice() { return m_Device; }
		const vk::Device& GetDevice() const { return m_Device; }
		vk::Queue& GetGraphicsQueue() { return m_GraphicsQueue; }
		const vk::Queue& GetGraphicsQueue() const { return m_GraphicsQueue; }

		void SetInstance(const vk::Instance& instance) { m_Instance = instance; }
		void SetPhysicalDevice(const vk::PhysicalDevice& physicalDevice) { m_PhysicalDevice = physicalDevice; }
		void SetDevice(const vk::Device& device) { m_Device = device; }
		void SetGraphicsQueue(const vk::Queue& graphicsQueue) { m_GraphicsQueue = graphicsQueue; }

	private:
		vk::Instance m_Instance = vk::Instance();
		vk::PhysicalDevice m_PhysicalDevice = vk::PhysicalDevice();
		vk::Device m_Device = vk::Device();
		vk::Queue m_GraphicsQueue = vk::Queue();

		static std::unique_ptr<VulkanCore> s_Instance;
	};
}
