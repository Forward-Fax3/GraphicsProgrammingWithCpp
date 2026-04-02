#include "VulkanUniformBuffer.hpp"
#include "VulkanCore.hpp"


namespace OWC::Graphics
{
	//--------------------------------------------------------
	// VulkanUniformBuffer
	//--------------------------------------------------------

	VulkanUniformBuffer::VulkanUniformBuffer(const uSize size)
	{
		const auto& vkCore = VulkanCore::GetInstance();
		const auto& allocator = vkCore.GetVulkanMemoryAllocator();

		// Create uniform buffers for each frame in flight and allocate memory
		const vk::DeviceSize bufferSize = size;

		m_UniformBuffers.reserve(vkCore.GetNumberOfFramesInFlight());
		m_UniformBuffersMemory.reserve(vkCore.GetNumberOfFramesInFlight());

		const auto& queueIndices = vkCore.GetAllUniqueQueuesIndices();

		for (uSize i = 0; i < vkCore.GetNumberOfFramesInFlight(); i++)
		{
			vk::BufferCreateInfo bufferInfo = vk::BufferCreateInfo()
				.setSize(bufferSize)
				.setUsage(vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst)
				.setSharingMode(vk::SharingMode::eConcurrent)
				.setQueueFamilyIndices(queueIndices);

			vma::AllocationCreateInfo allocInfo = vma::AllocationCreateInfo()
				.setUsage(vma::MemoryUsage::eAutoPreferDevice)
				.setFlags(vma::AllocationCreateFlagBits::eDedicatedMemory);

			vma::AllocationInfo allocationInfo;
			auto [allocation, buffer] = allocator.createBuffer(bufferInfo, allocInfo, allocationInfo);

			m_UniformBuffers.emplace_back(buffer);
			m_UniformBuffersMemory.emplace_back(allocation);
		}
	}

	VulkanUniformBuffer::~VulkanUniformBuffer()
	{
		const auto& vkCore = VulkanCore::GetInstance();
		const auto& allocator = vkCore.GetVulkanMemoryAllocator();
		for (uSize i = 0; i < m_UniformBuffers.size(); i++)
			allocator.destroyBuffer(m_UniformBuffers[i], m_UniformBuffersMemory[i]);
	}

	void VulkanUniformBuffer::UpdateBufferData(std::span<const std::byte> data)
	{
		const auto& vkCore = VulkanCore::GetInstance();
		const auto& allocator = vkCore.GetVulkanMemoryAllocator();
		const uSize currentFrame = vkCore.GetCurrentFrameIndex();

		const auto bufferCreateInfo = vk::BufferCreateInfo()
			.setSize(data.size())
			.setUsage(vk::BufferUsageFlagBits::eTransferSrc)
			.setSharingMode(vk::SharingMode::eExclusive);

		constexpr auto allocCreateInfo = vma::AllocationCreateInfo()
			.setUsage(vma::MemoryUsage::eAutoPreferHost)
			.setFlags(vma::AllocationCreateFlagBits::eHostAccessSequentialWrite | vma::AllocationCreateFlagBits::eMapped);

		vma::AllocationInfo stagingBufferAllocationInfo;
		const auto [stagingBufferAllocation, stagingBuffer] = allocator.createBuffer(bufferCreateInfo, allocCreateInfo, stagingBufferAllocationInfo);

		std::memcpy(stagingBufferAllocationInfo.pMappedData, data.data(), data.size());

		const auto& cmdTransBuf = vkCore.GetSingleTimeTransferCommandBuffer();
		cmdTransBuf.begin(vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));

		const auto copyBufferSize = vk::BufferCopy2()
				.setSrcOffset(0)
				.setDstOffset(0)
				.setSize(data.size());
		const auto copyBufferInfo = vk::CopyBufferInfo2()
			.setSrcBuffer(stagingBuffer)
			.setDstBuffer(m_UniformBuffers[currentFrame])
			.setRegions(copyBufferSize);

		cmdTransBuf.copyBuffer2(copyBufferInfo);

		const auto transferBufferMemoryBarrier = vk::BufferMemoryBarrier2()
			.setSrcAccessMask(vk::AccessFlagBits2::eTransferWrite)
			.setDstAccessMask(vk::AccessFlagBits2::eNone)
			.setSrcStageMask(vk::PipelineStageFlagBits2::eTransfer)
			.setDstStageMask(vk::PipelineStageFlagBits2::eNone)
			.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
			.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
			.setBuffer(m_UniformBuffers[currentFrame])
			.setOffset(0)
			.setSize(data.size());

		cmdTransBuf.pipelineBarrier2(vk::DependencyInfo()
			.setBufferMemoryBarriers(transferBufferMemoryBarrier)
		);
		cmdTransBuf.end();

		const auto semaphore = vkCore.GetSingleSemaphore();

		const auto cmdTransferSubmit = vk::CommandBufferSubmitInfo().setCommandBuffer(cmdTransBuf);
		const auto semaphoreSubmit = vk::SemaphoreSubmitInfo()
			.setSemaphore(semaphore)
			.setStageMask(vk::PipelineStageFlagBits2::eTransfer);

		const auto transferSubmitInfo = vk::SubmitInfo2()
			.setCommandBufferInfos(cmdTransferSubmit)
			.setSignalSemaphoreInfos(semaphoreSubmit);

		vkCore.GetTransferQueue().submit2(transferSubmitInfo);

		const auto cmdGraphicsBuffer = vkCore.GetSingleTimeGraphicsCommandBuffer();
		cmdGraphicsBuffer.begin(vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));

		const auto GraphicsBufferMemoryBarrier = vk::BufferMemoryBarrier2()
			.setSrcAccessMask(vk::AccessFlagBits2::eNone)
			.setDstAccessMask(vk::AccessFlagBits2::eUniformRead)
			.setSrcStageMask(vk::PipelineStageFlagBits2::eNone)
			.setDstStageMask(vk::PipelineStageFlagBits2::eVertexShader | vk::PipelineStageFlagBits2::eFragmentShader) // assuming the uniform buffer is used in both vertex and fragment shaders, adjust as necessary
			.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
			.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
			.setBuffer(m_UniformBuffers[currentFrame])
			.setOffset(0)
			.setSize(data.size());

		cmdGraphicsBuffer.pipelineBarrier2(vk::DependencyInfo()
			.setBufferMemoryBarriers(GraphicsBufferMemoryBarrier)
		);

		cmdGraphicsBuffer.end();

		const auto cmdGraphicsSubmitInfo = vk::CommandBufferSubmitInfo().setCommandBuffer(cmdGraphicsBuffer);

		const auto graphicsSemaphore = vk::SemaphoreSubmitInfo()
			.setSemaphore(semaphore)
			.setStageMask(vk::PipelineStageFlagBits2::eVertexShader | vk::PipelineStageFlagBits2::eFragmentShader);

		vkCore.GetGraphicsQueue().submit2(vk::SubmitInfo2()
			.setCommandBufferInfos(cmdGraphicsSubmitInfo)
			.setWaitSemaphoreInfos(graphicsSemaphore)
		);

		VulkanCore::GetInstance().AddVulkanEndOfFrameCleanUpFunction([&vkCore, &allocator, stagingBuffer, stagingBufferAllocation, semaphore, cmdTransBuf, cmdGraphicsBuffer]()
		{
			const auto& device = vkCore.GetDevice();

			device.freeCommandBuffers(vkCore.GetTransferCommandPool(), cmdTransBuf);
			device.freeCommandBuffers(vkCore.GetGraphicsCommandPool(), cmdGraphicsBuffer);

			allocator.destroyBuffer(stagingBuffer, stagingBufferAllocation);
			vkCore.GetDevice().destroySemaphore(semaphore);
		});
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

	VulkanTextureBuffer::VulkanTextureBuffer(const u32 width, const u32 height)
		: m_Width(width), m_Height(height)
	{
		InitializeTexture();
	}

	VulkanTextureBuffer::~VulkanTextureBuffer()
	{
		const auto& vkCore = VulkanCore::GetConstInstance();
		const auto& device = vkCore.GetDevice();
		const auto& allocator = vkCore.GetVulkanMemoryAllocator();

		device.destroySampler(m_TextureSampler);
		device.destroyImageView(m_TextureImageView);
		allocator.destroyImage(m_TextureImage, m_TextureImageMemory);
	}

	void VulkanTextureBuffer::UpdateBufferData(const std::vector<Vec4>& data)
	{
		const auto& vkCore = VulkanCore::GetInstance();
		const auto& allocator = vkCore.GetVulkanMemoryAllocator();

		// Create staging buffer
		const vk::DeviceSize imageSize = static_cast<uSize>(m_Width) * static_cast<uSize>(m_Height) * sizeof(Vec4);
		const auto bufferInfo = vk::BufferCreateInfo()
			.setSize(imageSize)
			.setUsage(vk::BufferUsageFlagBits::eTransferSrc)
			.setSharingMode(vk::SharingMode::eExclusive);

		constexpr auto allocCreateInfo = vma::AllocationCreateInfo()
			.setUsage(vma::MemoryUsage::eAutoPreferHost)
			.setFlags(vma::AllocationCreateFlagBits::eHostAccessSequentialWrite); // explicit

		vma::AllocationInfo stagingBufferAllocationInfo;
		const auto [allocation, stagingBuffer] = allocator.createBuffer(bufferInfo, allocCreateInfo, stagingBufferAllocationInfo);

		std::memcpy(stagingBufferAllocationInfo.pMappedData, data.data(), imageSize);

		// Copy staging buffer to texture image and transition image layout
		const auto& cmdTransBuf = vkCore.GetSingleTimeTransferCommandBuffer();
		cmdTransBuf.begin(vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));

		const std::array imageMemoryBarrierBegin = {
			vk::ImageMemoryBarrier2()
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
		};

		cmdTransBuf.pipelineBarrier2(vk::DependencyInfo()
			.setImageMemoryBarriers(imageMemoryBarrierBegin)
		);

		cmdTransBuf.copyBufferToImage(
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

		const std::array imageMemoryBarrierEnd = {
			vk::ImageMemoryBarrier2()
				.setSrcStageMask(vk::PipelineStageFlagBits2::eTransfer)
				.setDstStageMask(vk::PipelineStageFlagBits2::eNone)
				.setSrcAccessMask(vk::AccessFlagBits2::eTransferWrite)
				.setDstAccessMask(vk::AccessFlagBits2::eNone)
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
		};

		cmdTransBuf.pipelineBarrier2(vk::DependencyInfo()
			.setImageMemoryBarriers(imageMemoryBarrierEnd)
		);

		cmdTransBuf.end();

		auto semaphore = vkCore.GetSingleSemaphore();

		const auto cmdTransferSubmitInfo = vk::CommandBufferSubmitInfo()
			.setCommandBuffer(cmdTransBuf);
		const auto semaphoreTransferInfo = vk::SemaphoreSubmitInfo()
			.setSemaphore(semaphore)
			.setStageMask(vk::PipelineStageFlagBits2::eAllTransfer);

		const auto submitTransferInfo = vk::SubmitInfo2()
			.setCommandBufferInfos(cmdTransferSubmitInfo)
			.setSignalSemaphoreInfos(semaphoreTransferInfo);
		vkCore.GetTransferQueue().submit2(submitTransferInfo);

		const auto cmdGraphicsBuf = vkCore.GetSingleTimeGraphicsCommandBuffer();
		cmdGraphicsBuf.begin(vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
		const std::array graphicsImageBuffer = {
			vk::ImageMemoryBarrier2()
				.setSrcStageMask(vk::PipelineStageFlagBits2::eNone)
				.setDstStageMask(vk::PipelineStageFlagBits2::eFragmentShader | vk::PipelineStageFlagBits2::eBottomOfPipe)
				.setSrcAccessMask(vk::AccessFlagBits2::eNone)
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
		};
		cmdGraphicsBuf.pipelineBarrier2(vk::DependencyInfo()
			.setImageMemoryBarriers(graphicsImageBuffer)
		);
		cmdGraphicsBuf.end();

		const auto cmdInfo = vk::CommandBufferSubmitInfo().setCommandBuffer(cmdGraphicsBuf);
		const auto semaphoreInfo = vk::SemaphoreSubmitInfo()
			.setSemaphore(semaphore)
			.setStageMask(vk::PipelineStageFlagBits2::eFragmentShader | vk::PipelineStageFlagBits2::eBottomOfPipe);
		const auto submitGraphicsInfo = vk::SubmitInfo2()
			.setCommandBufferInfos(cmdInfo)
			.setWaitSemaphoreInfos(semaphoreInfo);
		vkCore.GetGraphicsQueue().submit2(submitGraphicsInfo);

		VulkanCore::GetInstance().AddVulkanEndOfFrameCleanUpFunction([&vkCore, &allocator, cmdTransBuf, cmdGraphicsBuf, semaphore, stagingBuffer, allocation]() -> void
		{
			const auto& device = vkCore.GetDevice();

			device.freeCommandBuffers(vkCore.GetTransferCommandPool(), cmdTransBuf);
			device.freeCommandBuffers(vkCore.GetGraphicsCommandPool(), cmdGraphicsBuf);
			device.destroySemaphore(semaphore);

			// Clean up staging buffer
			allocator.destroyBuffer(stagingBuffer, allocation);
		});
	}

	void VulkanTextureBuffer::InitializeTexture()
	{
		const auto& vkCore = VulkanCore::GetInstance();
		const auto& device = vkCore.GetDevice();
		const auto& allocator = vkCore.GetVulkanMemoryAllocator();

		const auto createInfo = vk::ImageCreateInfo()
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
			.setSharingMode(vk::SharingMode::eConcurrent)
			.setInitialLayout(vk::ImageLayout::eUndefined);

		constexpr auto vmaCreateInfo = vma::AllocationCreateInfo()
			.setUsage(vma::MemoryUsage::eAutoPreferDevice)
			.setFlags(vma::AllocationCreateFlagBits::eDedicatedMemory);

		// Create image
		std::tie(m_TextureImageMemory, m_TextureImage) = allocator.createImage(createInfo, vmaCreateInfo);

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

		const f32 maxAnisotropy = vkCore.GetPhysicalDev().getProperties().limits.maxSamplerAnisotropy;

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

	//--------------------------------------------------------
	// VulkanDynamicTextureBuffer
	//--------------------------------------------------------

	VulkanDynamicTextureBuffer::VulkanDynamicTextureBuffer(const u32 width, const u32 height)
		: m_Width(width), m_Height(height)
	{
		InitializeTexture();
	}

	VulkanDynamicTextureBuffer::~VulkanDynamicTextureBuffer()
	{
		const auto& vkCore = VulkanCore::GetConstInstance();
		const auto& device = vkCore.GetDevice();
		const auto& allocator = vkCore.GetVulkanMemoryAllocator();

		for (uSize i = 0; i < m_TextureImage.size(); i++)
		{
			device.destroySampler(m_TextureSampler[i]);
			device.destroyImageView(m_TextureImageView[i]);

			allocator.destroyImage(m_TextureImage[i], m_TextureImageMemory[i]);
		}
	}

	void VulkanDynamicTextureBuffer::UpdateBufferData(const std::vector<Vec4>& data)
	{
		const auto& vkCore = VulkanCore::GetInstance();
		const auto& allocator = vkCore.GetVulkanMemoryAllocator();
		const uSize currentFrame = vkCore.GetCurrentFrameIndex();

		// Create staging buffer
		const vk::DeviceSize imageSize = static_cast<uSize>(m_Width) * static_cast<uSize>(m_Height) * sizeof(Vec4);

		const auto bufferCreateInfo = vk::BufferCreateInfo()
													   .setSize(imageSize)
													   .setUsage(vk::BufferUsageFlagBits::eTransferSrc)
													   .setSharingMode(vk::SharingMode::eExclusive);

		constexpr auto allocationCreateInfo = vma::AllocationCreateInfo()
		.setUsage(vma::MemoryUsage::eAuto)
		.setFlags(vma::AllocationCreateFlagBits::eHostAccessSequentialWrite | vma::AllocationCreateFlagBits::eMapped);

		vma::AllocationInfo allocInfo;
		const auto [allocation, stagingBuffer] = allocator.createBuffer(bufferCreateInfo, allocationCreateInfo, allocInfo);

		std::memcpy(allocInfo.pMappedData, data.data(), imageSize);

		// Copy staging buffer to texture image and transition image layout
		const auto& cmdTransBuf = vkCore.GetSingleTimeTransferCommandBuffer();
		cmdTransBuf.begin(vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));

		const std::array imageMemoryBarrierBegin = {
			vk::ImageMemoryBarrier2()
				.setSrcStageMask(vk::PipelineStageFlagBits2::eTopOfPipe)
				.setDstStageMask(vk::PipelineStageFlagBits2::eTransfer)
				.setSrcAccessMask(vk::AccessFlagBits2::eNone)
				.setDstAccessMask(vk::AccessFlagBits2::eTransferWrite)
				.setOldLayout(vk::ImageLayout::eUndefined)
				.setNewLayout(vk::ImageLayout::eTransferDstOptimal)
				.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
				.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
				.setImage(m_TextureImage[currentFrame])
				.setSubresourceRange(vk::ImageSubresourceRange()
					.setAspectMask(vk::ImageAspectFlagBits::eColor)
					.setBaseMipLevel(0)
					.setLevelCount(1)
					.setBaseArrayLayer(0)
					.setLayerCount(1)
				)
		};

		cmdTransBuf.pipelineBarrier2(vk::DependencyInfo()
			.setImageMemoryBarriers(imageMemoryBarrierBegin)
		);

		cmdTransBuf.copyBufferToImage(
			stagingBuffer,
			m_TextureImage[currentFrame],
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

		const std::array imageMemoryBarrierEnd = {
			vk::ImageMemoryBarrier2()
				.setSrcStageMask(vk::PipelineStageFlagBits2::eTransfer)
				.setDstStageMask(vk::PipelineStageFlagBits2::eNone)
				.setSrcAccessMask(vk::AccessFlagBits2::eTransferWrite)
				.setDstAccessMask(vk::AccessFlagBits2::eNone)
				.setOldLayout(vk::ImageLayout::eTransferDstOptimal)
				.setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
				.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
				.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
				.setImage(m_TextureImage[currentFrame])
				.setSubresourceRange(vk::ImageSubresourceRange()
					.setAspectMask(vk::ImageAspectFlagBits::eColor)
					.setBaseMipLevel(0)
					.setLevelCount(1)
					.setBaseArrayLayer(0)
					.setLayerCount(1)
				)
		};

		cmdTransBuf.pipelineBarrier2(vk::DependencyInfo()
			.setImageMemoryBarriers(imageMemoryBarrierEnd)
		);

		cmdTransBuf.end();

		auto semaphore = vkCore.GetSingleSemaphore();

		const auto cmdTransferSubmitInfo = vk::CommandBufferSubmitInfo()
			.setCommandBuffer(cmdTransBuf);
		const auto semaphoreTransferInfo = vk::SemaphoreSubmitInfo()
			.setSemaphore(semaphore)
			.setStageMask(vk::PipelineStageFlagBits2::eAllTransfer);

		const auto submitTransferInfo = vk::SubmitInfo2()
			.setCommandBufferInfos(cmdTransferSubmitInfo)
			.setSignalSemaphoreInfos(semaphoreTransferInfo);
		vkCore.GetTransferQueue().submit2(submitTransferInfo);

		const auto cmdGraphicsBuf = vkCore.GetSingleTimeGraphicsCommandBuffer();
		cmdGraphicsBuf.begin(vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
		const std::array graphicsImageBuffer = {
			vk::ImageMemoryBarrier2()
				.setSrcStageMask(vk::PipelineStageFlagBits2::eNone)
				.setDstStageMask(vk::PipelineStageFlagBits2::eFragmentShader | vk::PipelineStageFlagBits2::eBottomOfPipe)
				.setSrcAccessMask(vk::AccessFlagBits2::eNone)
				.setDstAccessMask(vk::AccessFlagBits2::eShaderRead)
				.setOldLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
				.setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
				.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
				.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
				.setImage(m_TextureImage[currentFrame])
				.setSubresourceRange(vk::ImageSubresourceRange()
					.setAspectMask(vk::ImageAspectFlagBits::eColor)
					.setBaseMipLevel(0)
					.setLevelCount(1)
					.setBaseArrayLayer(0)
					.setLayerCount(1)
				)
		};
		cmdGraphicsBuf.pipelineBarrier2(vk::DependencyInfo()
			.setImageMemoryBarriers(graphicsImageBuffer)
		);
		cmdGraphicsBuf.end();

		const auto cmdInfo = vk::CommandBufferSubmitInfo().setCommandBuffer(cmdGraphicsBuf);
		const auto semaphoreInfo = vk::SemaphoreSubmitInfo()
			.setSemaphore(semaphore)
			.setStageMask(vk::PipelineStageFlagBits2::eFragmentShader | vk::PipelineStageFlagBits2::eBottomOfPipe);
		const auto submitGraphicsInfo = vk::SubmitInfo2()
			.setCommandBufferInfos(cmdInfo)
			.setWaitSemaphoreInfos(semaphoreInfo);
		vkCore.GetGraphicsQueue().submit2(submitGraphicsInfo);

		VulkanCore::GetInstance().AddVulkanEndOfFrameCleanUpFunction([&vkCore, &allocator, cmdTransBuf, cmdGraphicsBuf, semaphore, stagingBuffer, allocation]() -> void
		{
			const auto& device = vkCore.GetDevice();

			device.freeCommandBuffers(vkCore.GetTransferCommandPool(), cmdTransBuf);
			device.freeCommandBuffers(vkCore.GetGraphicsCommandPool(), cmdGraphicsBuf);
			device.destroySemaphore(semaphore);

			// Clean up staging buffer
			allocator.destroyBuffer(stagingBuffer, allocation);
		});
	}

	void VulkanDynamicTextureBuffer::InitializeTexture()
	{
		const auto& vkCore = VulkanCore::GetConstInstance();
		const auto& device = vkCore.GetDevice();
		const auto& allocator = vkCore.GetVulkanMemoryAllocator();

		m_TextureImage.resize(vkCore.GetNumberOfFramesInFlight());
		m_TextureImageMemory.resize(vkCore.GetNumberOfFramesInFlight());
		m_TextureImageView.resize(vkCore.GetNumberOfFramesInFlight());
		m_TextureSampler.resize(vkCore.GetNumberOfFramesInFlight());

		const f32 maxAnisotropy = vkCore.GetPhysicalDev().getProperties().limits.maxSamplerAnisotropy;
		const auto& allQueueFamilyIndices = vkCore.GetAllUniqueQueuesIndices();

		const auto imageCreateInfo = vk::ImageCreateInfo()
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
				.setSharingMode(vk::SharingMode::eConcurrent)
				.setInitialLayout(vk::ImageLayout::eUndefined)
				.setQueueFamilyIndices(allQueueFamilyIndices);

		constexpr auto allocInfo = vma::AllocationCreateInfo()
			.setUsage(vma::MemoryUsage::eAutoPreferDevice)
			.setFlags(vma::AllocationCreateFlagBits::eDedicatedMemory);

		for (uSize i = 0; i < vkCore.GetNumberOfFramesInFlight(); i++)
		{
			// Create image
			std::tie(m_TextureImageMemory[i], m_TextureImage[i]) = allocator.createImage(imageCreateInfo, allocInfo);

			// Create image view
			m_TextureImageView[i] = device.createImageView(vk::ImageViewCreateInfo()
				.setImage(m_TextureImage[i])
				.setViewType(vk::ImageViewType::e2D)
				.setFormat(vk::Format::eR32G32B32A32Sfloat)
				.setSubresourceRange(vk::ImageSubresourceRange()
					.setAspectMask(vk::ImageAspectFlagBits::eColor)
					.setBaseMipLevel(0)
					.setLevelCount(1)
					.setBaseArrayLayer(0)
					.setLayerCount(1)));

			// Create sampler
			m_TextureSampler[i] = device.createSampler(vk::SamplerCreateInfo()
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
}
