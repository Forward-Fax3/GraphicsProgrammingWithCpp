//
// Created by forwardfax3 on 17/04/2026.
//

#include "VulkanTLAS.hpp"
#include "VulkanSceneMesh.hpp"
#include "VulkanUniformBuffer.hpp"


namespace OWC
{
    VulkanTLAS::~VulkanTLAS()
    {
        if (m_TLAS)
        {
            const auto& vkCore = Graphics::VulkanCore::GetConstInstance();
            vkCore.GetDevice().destroyAccelerationStructureKHR(m_TLAS);
        }
    }

    void VulkanTLAS::AddInstance(const Mat4& transform, const std::shared_ptr<SceneMesh>& mesh)
    {
        const auto vulkanMesh = std::dynamic_pointer_cast<VulkanSceneMesh>(mesh);

        m_BLASInstances.emplace_back(
            ConvertMat4ToVulkanTransform(transform),
            vulkanMesh->GetIndex(),
            0xFF,
            0,
            vk::GeometryInstanceFlagBitsKHR::eTriangleCullDisable,
            vulkanMesh->GetAccelerationStructureBuffer()->GetBufferDeviceAddress()
        );
    }

    void VulkanTLAS::CreateTLAS()
    {
        using namespace OWC::Graphics;

        const auto& vkCore = VulkanCore::GetConstInstance();
        const auto& device = vkCore.GetDevice();

        // nVidia vk_raytracing_tutorial helper func
        auto alignUp = [](auto value, size_t alignment) noexcept { return ((value + alignment - 1) & ~(alignment - 1)); };

        VulkanGeneralBuffer buffer(m_BLASInstances.size() * sizeof(vk::AccelerationStructureInstanceKHR));
        buffer.UpdateBufferData(std::bit_cast<const u8*>(m_BLASInstances.data()));

        auto accelerationStructureInstanceData = vk::AccelerationStructureGeometryInstancesDataKHR()
            .setData(vk::DeviceOrHostAddressConstKHR().setDeviceAddress(buffer.GetBufferDeviceAddress()));

        auto accelerationStructureGeometry = vk::AccelerationStructureGeometryKHR()
            .setGeometry(accelerationStructureInstanceData)
            .setGeometryType(vk::GeometryTypeKHR::eInstances);

        auto accelerationStructureBuildRangeInfo = vk::AccelerationStructureBuildRangeInfoKHR()
            .setPrimitiveCount(static_cast<u32>(m_BLASInstances.size()));

        auto buildInfo = vk::AccelerationStructureBuildGeometryInfoKHR()
            .setType(vk::AccelerationStructureTypeKHR::eTopLevel)
            .setFlags(vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace)
            .setGeometries(accelerationStructureGeometry);

        std::array<u32, 1> primitiveCount = { static_cast<u32>(m_BLASInstances.size()) };

        vk::AccelerationStructureBuildSizesInfoKHR buildSizes;
        device.getAccelerationStructureBuildSizesKHR(
            vk::AccelerationStructureBuildTypeKHR::eDevice,
            &buildInfo,
            primitiveCount.data(),
            &buildSizes
        );

        const vk::DeviceSize scratchBufferSize = alignUp(buildSizes.buildScratchSize, vkCore.GetAccelerationStructureProperties().minAccelerationStructureScratchOffsetAlignment);

        VulkanGeneralBuffer scratchBuffer(scratchBufferSize);
        m_TLASBuffer = std::make_shared<VulkanGeneralBuffer>(buildSizes.accelerationStructureSize);

        const auto accelerationStructureCreateInfo = vk::AccelerationStructureCreateInfoKHR()
            .setType(vk::AccelerationStructureTypeKHR::eTopLevel)
            .setSize(buildSizes.accelerationStructureSize)
            .setBuffer(m_TLASBuffer->GetBuffer());

        m_TLAS = device.createAccelerationStructureKHR(accelerationStructureCreateInfo);

        buildInfo.setScratchData(vk::DeviceOrHostAddressKHR().setDeviceAddress(scratchBuffer.GetBufferDeviceAddress()))
            .setDstAccelerationStructure(m_TLAS);

        auto cmd = vkCore.GetSingleTimeComputeCommandBuffer();

        const auto buildRangesPtr = &accelerationStructureBuildRangeInfo;
        cmd.begin(vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
        cmd.buildAccelerationStructuresKHR(1, &buildInfo, &buildRangesPtr);
        cmd.end();

        auto fence = device.createFence(vk::FenceCreateInfo());
        vkCore.GetComputeQueue().submit(vk::SubmitInfo().setCommandBuffers(cmd), fence);
        if(device.waitForFences(fence, VK_TRUE, std::numeric_limits<uint64_t>::max()) != vk::Result::eSuccess)
            Log<LogLevel::Critical>("Failed to wait for acceleration structure build fence");
        device.destroyFence(fence);
    }

    vk::TransformMatrixKHR VulkanTLAS::ConvertMat4ToVulkanTransform(const Mat4& transform)
    {
        static_assert(sizeof(Mat4) >= sizeof(vk::TransformMatrixKHR), "Mat4 must be at least as large as vk::TransformMatrixKHR to safely copy the data.");

        // Vulkan expects row-major order, so we need to transpose the matrix
        const Mat4 transposed = glm::transpose(transform);

        vk::TransformMatrixKHR vulkanTransform;
        std::memcpy(&vulkanTransform.matrix, &transposed, sizeof(vk::TransformMatrixKHR));
        return vulkanTransform;
    }
}
