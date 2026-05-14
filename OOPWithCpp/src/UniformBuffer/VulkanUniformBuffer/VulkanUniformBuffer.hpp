#pragma once
#include "Core.hpp"
#include "UniformBuffer.hpp"
#include "VulkanCore.hpp"

#include <vulkan/vulkan.hpp>


namespace OWC::Graphics
{
	class VulkanUniformBuffer : public UniformBuffer
	{
	public:
		VulkanUniformBuffer() = delete;
		explicit VulkanUniformBuffer(uSize size);
		~VulkanUniformBuffer() override;
		VulkanUniformBuffer(VulkanUniformBuffer&) = delete;
		VulkanUniformBuffer& operator=(VulkanUniformBuffer&) = delete;
		VulkanUniformBuffer(VulkanUniformBuffer&&) noexcept = delete;
		VulkanUniformBuffer& operator=(VulkanUniformBuffer&&) noexcept = delete;

		void UpdateBufferData(std::span<const std::byte> data) override;

		[[nodiscard]] vk::Buffer GetBuffer() const { return m_UniformBuffers[VulkanCore::GetConstInstance().GetCurrentFrameIndex()]; };
		[[nodiscard]] uSize GetBufferSize() const { return static_cast<uSize>(VulkanCore::GetConstInstance().GetDevice().getBufferMemoryRequirements(m_UniformBuffers[0]).size); }

		[[nodiscard]] std::vector<vk::Buffer> GetBuffers() const { return m_UniformBuffers; }

	private:
		std::vector<vk::Buffer> m_UniformBuffers;
		std::vector<vma::Allocation> m_UniformBuffersMemory;
	};

	class VulkanTextureBuffer : public TextureBuffer
	{
	public:
		VulkanTextureBuffer() = delete;
		explicit VulkanTextureBuffer(const ImageLoader& image);
		explicit VulkanTextureBuffer(u32 width, u32 height);
		~VulkanTextureBuffer() override;
		VulkanTextureBuffer(VulkanTextureBuffer&) = delete;
		VulkanTextureBuffer& operator=(const VulkanTextureBuffer&) = delete;
		VulkanTextureBuffer(VulkanTextureBuffer&&) noexcept = delete;
		VulkanTextureBuffer& operator=(VulkanTextureBuffer&&) noexcept = delete;

		void UpdateBufferData(const std::vector<Vec4>& data) override;

		[[nodiscard]] OWC_FORCE_INLINE vk::Image GetImage() const { return m_TextureImage; }
		[[nodiscard]] OWC_FORCE_INLINE vk::ImageView GetImageView() const { return m_TextureImageView; }
		[[nodiscard]] OWC_FORCE_INLINE vk::Sampler GetSampler() const { return m_TextureSampler; }
		[[nodiscard]] OWC_FORCE_INLINE u32 GetWidth() const { return m_Width; }
		[[nodiscard]] OWC_FORCE_INLINE u32 GetHeight() const { return m_Height; }

		[[nodiscard]] OWC_FORCE_INLINE vk::AccessFlags2 GetCurrentAccessFlags() const { return m_CurrentAccessFlags; }
		[[nodiscard]] OWC_FORCE_INLINE vk::PipelineStageFlags2 GetCurrentPipelineStageFlags() const { return m_CurrentPipelineStageFlags; }
		[[nodiscard]] OWC_FORCE_INLINE vk::ImageLayout GetCurrentLayout() const { return m_CurrentLayout; }
		OWC_FORCE_INLINE void SetCurrentAccessFlags(const vk::AccessFlags2 previousAccessFlags) { m_CurrentAccessFlags = previousAccessFlags; }
		OWC_FORCE_INLINE void SetCurrentPipelineStageFlags(const vk::PipelineStageFlags2 previousPipelineStageFlags) { m_CurrentPipelineStageFlags = previousPipelineStageFlags; }
		OWC_FORCE_INLINE void SetCurrentImageLayout(const vk::ImageLayout previousLayout) { m_CurrentLayout = previousLayout; }

	private:
		void InitializeTexture();

	private:
		vk::Image m_TextureImage = vk::Image();
		vma::Allocation m_TextureImageMemory = vma::Allocation();
		vk::ImageView m_TextureImageView = vk::ImageView();
		vk::Sampler m_TextureSampler = vk::Sampler();
		u32 m_Width = 0;
		u32 m_Height = 0;
		vk::AccessFlags2 m_CurrentAccessFlags = vk::AccessFlags2();
		vk::PipelineStageFlags2 m_CurrentPipelineStageFlags = vk::PipelineStageFlags2();
		vk::ImageLayout m_CurrentLayout = vk::ImageLayout();
	};

	class VulkanDynamicTextureBuffer : public DynamicTextureBuffer
	{
	public:
		VulkanDynamicTextureBuffer() = delete;
		explicit VulkanDynamicTextureBuffer(u32 width, u32 height);
		~VulkanDynamicTextureBuffer() override;
		VulkanDynamicTextureBuffer(VulkanDynamicTextureBuffer&) = delete;
		VulkanDynamicTextureBuffer& operator=(VulkanDynamicTextureBuffer&) = delete;
		VulkanDynamicTextureBuffer(VulkanDynamicTextureBuffer&&) noexcept = delete;
		VulkanDynamicTextureBuffer& operator=(VulkanDynamicTextureBuffer&&) noexcept = delete;
		void UpdateBufferData(const std::vector<Vec4>& data) override;

		[[nodiscard]] vk::Image GetImage() const { return m_TextureImage[VulkanCore::GetConstInstance().GetCurrentFrameIndex()]; }
		[[nodiscard]] vk::ImageView GetImageView() const { return m_TextureImageView[VulkanCore::GetConstInstance().GetCurrentFrameIndex()]; }
		[[nodiscard]] vk::Sampler GetSampler() const { return m_TextureSampler[VulkanCore::GetConstInstance().GetCurrentFrameIndex()]; }

		[[nodiscard]] const std::vector<vk::Image>& GetImages() const { return m_TextureImage; }
		[[nodiscard]] const std::vector<vk::ImageView>& GetImageViews() const { return m_TextureImageView; }
		[[nodiscard]] const std::vector<vk::Sampler>& GetSamplers() const { return m_TextureSampler; }

	private:
		void InitializeTexture();

	private:
		std::vector<vk::Image> m_TextureImage = {};
		std::vector<vma::Allocation> m_TextureImageMemory = {};
		std::vector<vk::ImageView> m_TextureImageView = {};
		std::vector<vk::Sampler> m_TextureSampler = {};
		u32 m_Width = 0;
		u32 m_Height = 0;
	};

	class VulkanGeneralBuffer : public GeneralBuffer
	{
		public:
		VulkanGeneralBuffer() = delete;
		explicit VulkanGeneralBuffer(uSize size);
		~VulkanGeneralBuffer() override;
		VulkanGeneralBuffer(VulkanGeneralBuffer&&) noexcept = delete;
		VulkanGeneralBuffer& operator=(VulkanGeneralBuffer&&) noexcept = delete;

		void UpdateBufferDataImpl(const u8* data, uSize count, uSize offset) override;

		[[nodiscard]] vk::Buffer GetBuffer() const { return m_Buffer; }
		[[nodiscard]] uSize GetBufferSize() const { return static_cast<uSize>(VulkanCore::GetConstInstance().GetDevice().getBufferMemoryRequirements(m_Buffer).size); }
		[[nodiscard]] vk::DeviceAddress GetBufferDeviceAddress() const { return m_BufferDeviceAddress; }

		[[nodiscard]] uSize GetDeviceBufferPtr() const override { return m_BufferDeviceAddress; }

	private:
		vk::Buffer m_Buffer = vk::Buffer();
		vk::DeviceAddress m_BufferDeviceAddress = vk::DeviceAddress();
		vma::Allocation m_BufferMemory = vma::Allocation();
		uSize m_BufferSize = 0;
	};
}
