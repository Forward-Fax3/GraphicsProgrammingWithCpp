//
// Created by forwardfax3 on 17/04/2026.
//

#include "VulkanTLAS.hpp"
#include "VulkanSceneMesh.hpp"
#include "VulkanUniformBuffer.hpp"


namespace OWC
{
    void VulkanTLAS::AddInstance(const Mat4& transform, const std::shared_ptr<SceneMesh>& mesh)
    {
        const auto vulkanMesh = std::dynamic_pointer_cast<VulkanSceneMesh>(mesh);

        m_BLASInstances.emplace_back(
            ConvertMat4ToVulkanTransform(transform),
            vulkanMesh->GetCustomInstanceIndex(),
            0xFF,
            0,
            vk::GeometryInstanceFlagBitsKHR::eTriangleCullDisable,
            vulkanMesh->GetAccelerationStructureBufferDeviceAddress()
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
            .setFlags(vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace | vk::BuildAccelerationStructureFlagBitsKHR::eAllowCompaction)
            .setGeometries(accelerationStructureGeometry);

        vk::AccelerationStructureBuildSizesInfoKHR buildSizes = device.getAccelerationStructureBuildSizesKHR(
            vk::AccelerationStructureBuildTypeKHR::eDevice,
            buildInfo,
            m_BLASInstances.size()
        );

        const vk::DeviceSize scratchBufferSize = alignUp(buildSizes.buildScratchSize, vkCore.GetAccelerationStructureProperties().minAccelerationStructureScratchOffsetAlignment);

        VulkanGeneralBuffer scratchBuffer(scratchBufferSize);
        const auto scratchTLASBuffer = std::make_shared<VulkanGeneralBuffer>(buildSizes.accelerationStructureSize);

        auto accelerationStructureCreateInfo = vk::AccelerationStructureCreateInfoKHR()
            .setType(vk::AccelerationStructureTypeKHR::eTopLevel)
            .setSize(buildSizes.accelerationStructureSize)
            .setBuffer(scratchTLASBuffer->GetBuffer());

        auto scratchTLAS = device.createAccelerationStructureKHR(accelerationStructureCreateInfo);

        buildInfo.setScratchData(vk::DeviceOrHostAddressKHR().setDeviceAddress(scratchBuffer.GetBufferDeviceAddress()))
            .setDstAccelerationStructure(scratchTLAS);

        auto cmd = vkCore.GetSingleTimeComputeCommandBuffer();
        constexpr auto barrierData = vk::MemoryBarrier2()
            .setSrcStageMask(vk::PipelineStageFlagBits2::eAccelerationStructureBuildKHR)
            .setSrcAccessMask(vk::AccessFlagBits2::eAccelerationStructureWriteKHR)
            .setDstStageMask(vk::PipelineStageFlagBits2::eAccelerationStructureBuildKHR)
            .setDstAccessMask(vk::AccessFlagBits2::eAccelerationStructureReadKHR);

        const auto buildRangesPtr = &accelerationStructureBuildRangeInfo;

        vk::raii::QueryPool queryPool(device, vk::QueryPoolCreateInfo()
            .setQueryType(vk::QueryType::eAccelerationStructureCompactedSizeKHR)
            .setQueryCount(1));

        cmd.begin(vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
        cmd.buildAccelerationStructuresKHR(buildInfo, buildRangesPtr);
        cmd.pipelineBarrier2(vk::DependencyInfo().setMemoryBarriers(barrierData));
        cmd.resetQueryPool(*queryPool, 0, 1);
        cmd.writeAccelerationStructuresPropertiesKHR(*scratchTLAS, vk::QueryType::eAccelerationStructureCompactedSizeKHR, *queryPool, 0);
        cmd.end();

        auto fence = device.createFence(vk::FenceCreateInfo());
        auto cmdSubmitInfo = vk::CommandBufferSubmitInfo().setCommandBuffer(cmd);
        vkCore.GetComputeQueue().submit2(vk::SubmitInfo2().setCommandBufferInfos(cmdSubmitInfo), *fence);
        if(device.waitForFences(*fence, VK_TRUE, std::numeric_limits<uint64_t>::max()) != vk::Result::eSuccess)
            Log<LogLevel::Critical>("Failed to wait for acceleration structure build fence");

        const auto [result, sizes] = (*device).getQueryPoolResults<vk::DeviceSize>(*queryPool, 0, 1, sizeof(vk::DeviceSize), sizeof(vk::DeviceSize));

        if (result != vk::Result::eSuccess)
        {
            Log<LogLevel::Error>("Failed to get query pool results for TLAS compaction size, will use originally created TLAS");
            m_TLASBuffer = scratchTLASBuffer;
            m_TLAS = std::move(scratchTLAS);
            return;
        }

        Log<LogLevel::Trace>("TLAS starting size {}, compaction size: {}", buildSizes.accelerationStructureSize, sizes[0]);

        m_TLASBuffer = std::make_shared<VulkanGeneralBuffer>(sizes[0]);
        accelerationStructureCreateInfo.setSize(sizes[0]).setBuffer(m_TLASBuffer->GetBuffer());
        m_TLAS = device.createAccelerationStructureKHR(accelerationStructureCreateInfo);

        const auto copyAccelerationStructureInfo = vk::CopyAccelerationStructureInfoKHR()
            .setSrc(scratchTLAS)
            .setDst(m_TLAS)
            .setMode(vk::CopyAccelerationStructureModeKHR::eCompact);

        constexpr auto barrierDataCopy = vk::MemoryBarrier2()
            .setSrcStageMask(vk::PipelineStageFlagBits2::eAccelerationStructureBuildKHR)
            .setSrcAccessMask(vk::AccessFlagBits2::eAccelerationStructureWriteKHR)
            .setDstStageMask(vk::PipelineStageFlagBits2::eRayTracingShaderKHR)
            .setDstAccessMask(vk::AccessFlagBits2::eAccelerationStructureReadKHR);

        cmd = vkCore.GetSingleTimeComputeCommandBuffer();
        cmd.begin(vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
        cmd.copyAccelerationStructureKHR(copyAccelerationStructureInfo);
        cmd.pipelineBarrier2(vk::DependencyInfo().setMemoryBarriers(barrierDataCopy));
        cmd.end();

        device.resetFences(*fence);
        cmdSubmitInfo = vk::CommandBufferSubmitInfo().setCommandBuffer(cmd);
        vkCore.GetComputeQueue().submit2(vk::SubmitInfo2().setCommandBufferInfos(cmdSubmitInfo), *fence);
        if(device.waitForFences(*fence, VK_TRUE, std::numeric_limits<uint64_t>::max()) != vk::Result::eSuccess)
            Log<LogLevel::Critical>("Failed to wait for acceleration structure copy fence");
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
