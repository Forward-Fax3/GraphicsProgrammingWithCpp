#pragma once
#include "Image.hpp"

#include <vulkan/vulkan.hpp>


namespace OWC
{
    class VulkanImage : public Image
    {
    public:
        VulkanImage(ImageLoader image);
        ~VulkanImage() override = default;
		VulkanImage(const VulkanImage&) = delete;
		VulkanImage& operator=(const VulkanImage&) = delete;
		VulkanImage(VulkanImage&&) = delete;
		VulkanImage& operator=(VulkanImage&&) = delete;

    private:
		vk::Image m_Image;
		vk::ImageView m_ImageView;
		vk::DeviceMemory m_ImageMemory;
    };
}
