#include "UniformBuffer.hpp"
#include "VulkanUniformBuffer.hpp"


namespace OWC::Graphics
{
	std::shared_ptr<UniformBuffer> UniformBuffer::CreateUniformBuffer(uSize size)
	{
		// For now, only Vulkan is supported
		return std::make_shared<VulkanUniformBuffer>(size);
	}

	std::shared_ptr<TextureBuffer> TextureBuffer::CreateTextureBuffer(const ImageLoader& image)
	{
		// For now, only Vulkan is supported
		return std::make_shared<VulkanTextureBuffer>(image);
	}

	std::shared_ptr<TextureBuffer> TextureBuffer::CreateTextureBuffer(u32 width, u32 height)
	{
		// For now, only Vulkan is supported
		return std::make_shared<VulkanTextureBuffer>(width, height);
	}

	std::shared_ptr<OWC::Graphics::DynamicTextureBuffer> DynamicTextureBuffer::CreateDynamicTextureBuffer(u32 width, u32 height)
	{
		// For now, only Vulkan is supported
		return std::make_shared<VulkanDynamicTextureBuffer>(width, height);
	}
}
