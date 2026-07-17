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
        explicit VulkanSceneMesh(const tg3_model& model, i32 meshIndex, u32 customInstancesIndex, const std::shared_ptr<Graphics::GeneralBuffer>& GPUBuffer, std::vector<GPUGLTFData>& GPUData);
        ~VulkanSceneMesh() override = default;

        VulkanSceneMesh(const VulkanSceneMesh&) = delete;
        VulkanSceneMesh(VulkanSceneMesh&&) = delete;
        VulkanSceneMesh& operator=(const VulkanSceneMesh&) = delete;
        VulkanSceneMesh& operator=(VulkanSceneMesh&&) = delete;

        //[[nodiscard]] const AttributeData& GetAttributeData(const std::string& attributeName) const override;

        [[nodiscard]] u32 GetCustomInstanceIndex() const { return m_CustomInstanceIndex; }
        [[nodiscard]] vk::AccelerationStructureKHR GetAccelerationStructure() const { return m_AccelerationStructure; }
        [[nodiscard]] vk::Buffer GetAccelerationStructureBuffer() const { return m_Buffer; }
        [[nodiscard]] vk::DeviceAddress GetAccelerationStructureBufferDeviceAddress() const { return m_BufferDeviceAddress; }

    private:
        vma::raii::Buffer m_Buffer = nullptr;
        vk::DeviceAddress m_BufferDeviceAddress = vk::DeviceAddress();
        vk::raii::AccelerationStructureKHR m_AccelerationStructure = nullptr;
        u32 m_CustomInstanceIndex;
    };
} // OWC
