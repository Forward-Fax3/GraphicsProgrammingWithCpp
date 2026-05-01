//
// Created by forwardfax3 on 17/04/2026.
//

#pragma once
#include "BaseTLAS.hpp"
#include "VulkanUniformBuffer.hpp"

#include "vulkan/vulkan.hpp"


namespace OWC
{
    class VulkanTLAS : public BaseTLAS
    {
    public:
        VulkanTLAS() = default; // default init because we need to add all instances first
        ~VulkanTLAS() override;

        VulkanTLAS(const VulkanTLAS&) = delete;
        VulkanTLAS(VulkanTLAS&&) = delete;
        VulkanTLAS& operator=(const VulkanTLAS&) = delete;
        VulkanTLAS& operator=(VulkanTLAS&&) = delete;

        void AddInstance(const Mat4& transform, const std::shared_ptr<SceneMesh>& mesh) override;
        void CreateTLAS() override;

    private:
        [[nodiscard]] static vk::TransformMatrixKHR ConvertMat4ToVulkanTransform(const Mat4& transform);

    private:
        vk::AccelerationStructureKHR m_TLAS;
        std::shared_ptr<Graphics::VulkanGeneralBuffer> m_TLASBuffer;
        std::vector<vk::AccelerationStructureInstanceKHR> m_BLASInstances;
    };
}
