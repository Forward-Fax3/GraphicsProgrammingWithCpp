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
        ~VulkanTLAS() override = default;

        VulkanTLAS(const VulkanTLAS&) = delete;
        VulkanTLAS(VulkanTLAS&&) = delete;
        VulkanTLAS& operator=(const VulkanTLAS&) = delete;
        VulkanTLAS& operator=(VulkanTLAS&&) = delete;

        void AddInstance(const Mat4& transform, const std::shared_ptr<SceneMesh>& mesh) override;
        void CreateTLAS() override;

        const vk::raii::AccelerationStructureKHR& GetTLAS() const { return m_TLAS; }

    private:
        [[nodiscard]] static vk::TransformMatrixKHR ConvertMat4ToVulkanTransform(const Mat4& transform);

    private:
        vk::raii::AccelerationStructureKHR m_TLAS = nullptr;
        std::shared_ptr<Graphics::VulkanGeneralBuffer> m_TLASBuffer;
        std::vector<vk::AccelerationStructureInstanceKHR> m_BLASInstances;
    };
}
