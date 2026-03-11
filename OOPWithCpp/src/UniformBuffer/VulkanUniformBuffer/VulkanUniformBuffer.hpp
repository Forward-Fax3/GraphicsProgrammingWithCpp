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
		std::vector<vk::DeviceMemory> m_UniformBuffersMemory;
		std::vector<void*> m_MappedUniformBuffers;
	};

	class VulkanTextureBuffer : public TextureBuffer
	{
	public:
		VulkanTextureBuffer() = delete;
		explicit VulkanTextureBuffer(const ImageLoader& image);
		explicit VulkanTextureBuffer(u32 width, u32 height);
		~VulkanTextureBuffer() override;
		VulkanTextureBuffer(VulkanTextureBuffer&) = delete;
		VulkanTextureBuffer& operator=(VulkanTextureBuffer&) = delete;
		VulkanTextureBuffer(VulkanTextureBuffer&&) noexcept = delete;
		VulkanTextureBuffer& operator=(VulkanTextureBuffer&&) noexcept = delete;

		void UpdateBufferData(const std::vector<Vec4>& data) override;

		[[nodiscard]] vk::Image GetImage() const { return m_TextureImage; }
		[[nodiscard]] vk::ImageView GetImageView() const { return m_TextureImageView; }
		[[nodiscard]] vk::Sampler GetSampler() const { return m_TextureSampler; }

	private:
		void InitializeTexture();

	private:
		vk::Image m_TextureImage = vk::Image();
		vk::DeviceMemory m_TextureImageMemory = vk::DeviceMemory();
		vk::ImageView m_TextureImageView = vk::ImageView();
		vk::Sampler m_TextureSampler = vk::Sampler();
		u32 m_Width = 0;
		u32 m_Height = 0;
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
		std::vector<vk::DeviceMemory> m_TextureImageMemory = {};
		std::vector<vk::ImageView> m_TextureImageView = {};
		std::vector<vk::Sampler> m_TextureSampler = {};
		u32 m_Width = 0;
		u32 m_Height = 0;
	};
}
