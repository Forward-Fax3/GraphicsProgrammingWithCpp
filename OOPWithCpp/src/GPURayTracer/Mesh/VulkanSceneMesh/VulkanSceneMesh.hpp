//
// Created by forwardfax3 on 21/04/2026.
//

#pragma once
#include "SceneMesh.hpp"
#include "VulkanUniformBuffer.hpp"

#include <tiny_gltf_v3.h>


namespace OWC
{
    class VulkanSceneMesh : public SceneMesh
    {
    public:
        VulkanSceneMesh() = delete;
        explicit VulkanSceneMesh(const tg3_model& model, i32 meshIndex, const std::shared_ptr<Graphics::GeneralBuffer>& GPUBuffer);
        ~VulkanSceneMesh() override;

        VulkanSceneMesh(const VulkanSceneMesh&) = delete;
        VulkanSceneMesh(VulkanSceneMesh&&) = delete;
        VulkanSceneMesh& operator=(const VulkanSceneMesh&) = delete;
        VulkanSceneMesh& operator=(VulkanSceneMesh&&) = delete;

        [[nodiscard]] const AttributeData& GetAttributeData(const std::string& attributeName) const override;

        [[nodiscard]] i32 GetIndex() const { return index; }
        [[nodiscard]] vk::AccelerationStructureKHR GetAccelerationStructure() const { return m_AccelerationStructure; }
        [[nodiscard]] const std::shared_ptr<Graphics::VulkanGeneralBuffer>& GetAccelerationStructureBuffer() const { return m_AccelerationStructureBuffer; }

    private:
        std::shared_ptr<Graphics::VulkanGeneralBuffer> m_AccelerationStructureBuffer;
        vk::AccelerationStructureKHR m_AccelerationStructure;
        const tg3_model& m_Model;
        i32 index;
    };
} // OWC
