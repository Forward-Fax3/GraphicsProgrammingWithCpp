#include "VulkanUniformBuffer.hpp"
#include "VulkanCore.hpp"


namespace OWC::Graphics
{
	//--------------------------------------------------------
	// VulkanUniformBuffer
	//--------------------------------------------------------

	VulkanUniformBuffer::VulkanUniformBuffer(uSize size)
	{
		const auto& vkCore = VulkanCore::GetInstance();
		const auto& device = vkCore.GetDevice();

		// Create uniform buffers for each frame in flight and allocate memory
		vk::DeviceSize bufferSize = size;

		m_UniformBuffers.reserve(vkCore.GetNumberOfFramesInFlight());
		m_UniformBuffersMemory.reserve(vkCore.GetNumberOfFramesInFlight());
		m_MappedUniformBuffers.reserve(vkCore.GetNumberOfFramesInFlight());

		for (uSize i = 0; i < vkCore.GetNumberOfFramesInFlight(); i++)
		{
			vk::Buffer buffer = device.createBuffer(vk::BufferCreateInfo()
				.setSize(bufferSize)
				.setUsage(vk::BufferUsageFlagBits::eUniformBuffer)
				.setSharingMode(vk::SharingMode::eExclusive));
			m_UniformBuffers.emplace_back(buffer);

			vk::MemoryRequirements memRequirements = device.getBufferMemoryRequirements(buffer);

			vk::DeviceMemory memory = device.allocateMemory(vk::MemoryAllocateInfo()
				.setAllocationSize(memRequirements.size)
				.setMemoryTypeIndex(vkCore.FindMemoryType(memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)));
			m_UniformBuffersMemory.emplace_back(memory);

			device.bindBufferMemory(buffer, memory, 0);
			m_MappedUniformBuffers.emplace_back(device.mapMemory(memory, 0, bufferSize));
		}
	}

	VulkanUniformBuffer::~VulkanUniformBuffer()
	{
		const auto& vkCore = VulkanCore::GetInstance();
		const auto& device = vkCore.GetDevice();
		for (const auto& buffer : m_UniformBuffers)
			device.destroyBuffer(buffer);

		for (const auto& memory : m_UniformBuffersMemory)
		{
			device.unmapMemory(memory);
			device.freeMemory(memory);
		}
	}

	void VulkanUniformBuffer::UpdateBufferData(std::span<const std::byte> data)
	{
		const auto& vkCore = VulkanCore::GetInstance();
		uSize currentFrame = vkCore.GetCurrentFrameIndex();
		std::memcpy(m_MappedUniformBuffers[currentFrame], data.data(), data.size());

		// todo: add staging buffers
	}

	//--------------------------------------------------------
	// VulkanTextureBuffer
	//--------------------------------------------------------

	VulkanTextureBuffer::VulkanTextureBuffer(const ImageLoader& image)
		: m_Width(static_cast<u32>(image.GetWidth())), m_Height(static_cast<u32>(image.GetHeight()))
	{
		InitializeTexture();
		VulkanTextureBuffer::UpdateBufferData(image.GetImageData()); // using specifically VulkanTextureBuffer to remove static analysis warning
	}

	VulkanTextureBuffer::VulkanTextureBuffer(u32 width, u32 height)
		: m_Width(width), m_Height(height)
	{
		InitializeTexture();
	}

	VulkanTextureBuffer::~VulkanTextureBuffer()
	{
		const auto& device = VulkanCore::GetInstance().GetDevice();
		device.destroySampler(m_TextureSampler);
		device.destroyImageView(m_TextureImageView);
		device.destroyImage(m_TextureImage);
		device.freeMemory(m_TextureImageMemory);
	}

	void VulkanTextureBuffer::UpdateBufferData(const std::vector<Vec4>& data)
	{
		const auto& vkCore = VulkanCore::GetInstance();
		const auto& device = vkCore.GetDevice();

		vk::Buffer stagingBuffer;
		vk::DeviceMemory stagingBufferMemory;

		// Create staging buffer
		vk::DeviceSize imageSize = static_cast<uSize>(m_Width) * static_cast<uSize>(m_Height) * sizeof(Vec4);
		stagingBuffer = device.createBuffer(vk::BufferCreateInfo()
			.setSize(imageSize)
			.setUsage(vk::BufferUsageFlagBits::eTransferSrc)
			.setSharingMode(vk::SharingMode::eExclusive));

		vk::MemoryRequirements memRequirements = device.getBufferMemoryRequirements(stagingBuffer);
		stagingBufferMemory = device.allocateMemory(vk::MemoryAllocateInfo()
			.setAllocationSize(memRequirements.size)
			.setMemoryTypeIndex(vkCore.FindMemoryType(memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)));
		device.bindBufferMemory(stagingBuffer, stagingBufferMemory, 0);

		// Copy image data to staging buffer
		void* mappedData = device.mapMemory(stagingBufferMemory, 0, imageSize);

		std::memcpy(mappedData, data.data(), imageSize);

		// Copy staging buffer to texture image and transition image layout
		const auto& cmdBuf = vkCore.GetSingleTimeGraphicsCommandBuffer();
		cmdBuf.begin(vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));

		cmdBuf.pipelineBarrier2(vk::DependencyInfo()
			.setImageMemoryBarriers(vk::ImageMemoryBarrier2()
				.setSrcStageMask(vk::PipelineStageFlagBits2::eTopOfPipe)
				.setDstStageMask(vk::PipelineStageFlagBits2::eTransfer)
				.setSrcAccessMask(vk::AccessFlagBits2::eNone)
				.setDstAccessMask(vk::AccessFlagBits2::eTransferWrite)
				.setOldLayout(vk::ImageLayout::eUndefined)
				.setNewLayout(vk::ImageLayout::eTransferDstOptimal)
				.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
				.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
				.setImage(m_TextureImage)
				.setSubresourceRange(vk::ImageSubresourceRange()
					.setAspectMask(vk::ImageAspectFlagBits::eColor)
					.setBaseMipLevel(0)
					.setLevelCount(1)
					.setBaseArrayLayer(0)
					.setLayerCount(1)
				)
			)
		);

		cmdBuf.copyBufferToImage(
			stagingBuffer,
			m_TextureImage,
			vk::ImageLayout::eTransferDstOptimal,
			vk::BufferImageCopy()
				.setBufferOffset(0)
				.setBufferRowLength(0)
				.setBufferImageHeight(0)
				.setImageSubresource(vk::ImageSubresourceLayers()
					.setAspectMask(vk::ImageAspectFlagBits::eColor)
					.setMipLevel(0)
					.setBaseArrayLayer(0)
					.setLayerCount(1))
				.setImageOffset(vk::Offset3D{ 0, 0, 0 })
				.setImageExtent(vk::Extent3D{
					m_Width,
					m_Height,
					1
				}
			)
		);

		cmdBuf.pipelineBarrier2(vk::DependencyInfo()
			.setImageMemoryBarriers(vk::ImageMemoryBarrier2()
				.setSrcStageMask(vk::PipelineStageFlagBits2::eTransfer)
				.setDstStageMask(vk::PipelineStageFlagBits2::eFragmentShader | vk::PipelineStageFlagBits2::eBottomOfPipe)
				.setSrcAccessMask(vk::AccessFlagBits2::eTransferWrite)
				.setDstAccessMask(vk::AccessFlagBits2::eShaderRead)
				.setOldLayout(vk::ImageLayout::eTransferDstOptimal)
				.setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
				.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
				.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
				.setImage(m_TextureImage)
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

		// Submit command buffer and wait for completion
		// TODO: Use a better synchronization method
		vk::SubmitInfo submitInfo = vk::SubmitInfo().setCommandBuffers(cmdBuf);
		vk::Fence fence = device.createFence(vk::FenceCreateInfo());
		vkCore.GetGraphicsQueue().submit(submitInfo, fence);
		auto result = device.waitForFences(fence, vk::True, UINT64_MAX);
		if (result != vk::Result::eSuccess)
			Log<LogLevel::Critical>("Failed to wait for fence in VulkanTextureBuffer::UpdateBufferData");
		device.destroyFence(fence);
		device.freeCommandBuffers(vkCore.GetGraphicsCommandPool(), cmdBuf);

		// Clean up staging buffer
		device.unmapMemory(stagingBufferMemory);
		device.destroyBuffer(stagingBuffer);
		device.freeMemory(stagingBufferMemory);
	}

	void VulkanTextureBuffer::InitializeTexture()
	{
		const auto& vkCore = VulkanCore::GetInstance();
		const auto& device = vkCore.GetDevice();

		// Create image
		m_TextureImage = device.createImage(vk::ImageCreateInfo()
			.setImageType(vk::ImageType::e2D)
			.setFormat(vk::Format::eR32G32B32A32Sfloat)
			.setExtent(vk::Extent3D()
				.setWidth(m_Width)
				.setHeight(m_Height)
				.setDepth(1))
			.setMipLevels(1)
			.setArrayLayers(1)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setTiling(vk::ImageTiling::eOptimal)
			.setUsage(vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst)
			.setSharingMode(vk::SharingMode::eExclusive)
			.setInitialLayout(vk::ImageLayout::eUndefined));

		// Allocate image memory
		vk::MemoryRequirements memRequirements = device.getImageMemoryRequirements(m_TextureImage);

		m_TextureImageMemory = device.allocateMemory(vk::MemoryAllocateInfo()
			.setAllocationSize(memRequirements.size)
			.setMemoryTypeIndex(vkCore.FindMemoryType(memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal)));

		device.bindImageMemory(m_TextureImage, m_TextureImageMemory, 0);

		// Create image view
		m_TextureImageView = device.createImageView(vk::ImageViewCreateInfo()
			.setImage(m_TextureImage)
			.setViewType(vk::ImageViewType::e2D)
			.setFormat(vk::Format::eR32G32B32A32Sfloat)
			.setSubresourceRange(vk::ImageSubresourceRange()
				.setAspectMask(vk::ImageAspectFlagBits::eColor)
				.setBaseMipLevel(0)
				.setLevelCount(1)
				.setBaseArrayLayer(0)
				.setLayerCount(1)));

		f32 maxAnisotropy = vkCore.GetPhysicalDev().getProperties().limits.maxSamplerAnisotropy;

		// Create sampler
		m_TextureSampler = device.createSampler(vk::SamplerCreateInfo()
			.setMagFilter(vk::Filter::eLinear)
			.setMinFilter(vk::Filter::eLinear)
			.setAddressModeU(vk::SamplerAddressMode::eRepeat)
			.setAddressModeV(vk::SamplerAddressMode::eRepeat)
			.setAddressModeW(vk::SamplerAddressMode::eRepeat)
			.setAnisotropyEnable(vk::True)
			.setMaxAnisotropy(maxAnisotropy)
			.setBorderColor(vk::BorderColor::eIntOpaqueBlack)
			.setUnnormalizedCoordinates(vk::False)
			.setCompareEnable(vk::False)
			.setCompareOp(vk::CompareOp::eAlways)
			.setMipmapMode(vk::SamplerMipmapMode::eLinear)
			.setMipLodBias(0.0f)
			.setMinLod(0.0f)
			.setMaxLod(0.0f));
	}
}
