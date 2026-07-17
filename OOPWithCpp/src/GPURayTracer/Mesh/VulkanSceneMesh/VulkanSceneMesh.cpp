//
// Created by forwardfax3 on 21/04/2026.
//

#include "VulkanSceneMesh.hpp"
#include "VulkanUniformBuffer.hpp"
#include "Core.hpp"
#include "Log.hpp"
#include "VulkanCore.hpp"

#include <vulkan/vulkan.hpp>

#include "glm/gtc/type_ptr.hpp"

#include <algorithm>
#include <cstring>


#define DEFAULT_TRIANGLES static_cast<u32>(-1)


namespace OWC
{
    VulkanSceneMesh::VulkanSceneMesh(const tg3_model& model, const i32 meshIndex, u32 customInstancesIndex, const std::shared_ptr<Graphics::GeneralBuffer>& GPUBuffer, std::vector<GPUGLTFData>& GPUData)
        : m_CustomInstanceIndex(customInstancesIndex)
    {
        using namespace OWC::Graphics;
		const auto& vkCore = VulkanCore::GetConstInstance();
        const auto& device = vkCore.GetDevice();
        const auto& allocator = vkCore.GetVulkanMemoryAllocator();
        const auto& queueIndices = vkCore.GetAllUniqueQueuesIndices();

        const tg3_mesh& mesh = model.meshes[meshIndex];

        auto extractAttribute = [&model](const tg3_primitive& prim, const std::string& attributeName) -> AttributeData
        {
            for (u32 i = 0; i < prim.attributes_count; i++)
            {
                if (const auto& [key, accessorIndex] = prim.attributes[i]; std::string_view(key.data) == attributeName)
                {
                    const tg3_accessor& acc = model.accessors[accessorIndex];
                    const tg3_buffer_view& bv = model.buffer_views[acc.buffer_view];

                    return {
                        .offset = static_cast<u32>(bv.byte_offset + acc.byte_offset),
                        .count = static_cast<u32>(acc.count),
                        .byteStride = (bv.byte_stride ? static_cast<u32>(bv.byte_stride) : 0 /* FIXME: 0 for now */),
                        .bufferIndex = static_cast<u32>(bv.buffer),
                        .hasData = true
                    };
                }
            }
            return {};
        };

        auto computeMaxIndex = [&model](const tg3_accessor& indexAccessor, const u32 fallbackMaxVertex) -> u32
        {
            if (indexAccessor.buffer_view < 0)
                return fallbackMaxVertex;

            const tg3_buffer_view& bufferView = model.buffer_views[indexAccessor.buffer_view];
            if (bufferView.buffer < 0)
                return fallbackMaxVertex;

            const tg3_buffer& buffer = model.buffers[bufferView.buffer];
            if (!buffer.data.data || buffer.data.count == 0)
            {
                return fallbackMaxVertex;
            }

            const uint64_t baseOffset = bufferView.byte_offset + indexAccessor.byte_offset;
            uint32_t componentSize = 0;
            switch (indexAccessor.component_type)
            {
                case TG3_COMPONENT_TYPE_UNSIGNED_BYTE:
                    componentSize = sizeof(uint8_t);
                    break;
                case TG3_COMPONENT_TYPE_UNSIGNED_SHORT:
                    componentSize = sizeof(uint16_t);
                    break;
                case TG3_COMPONENT_TYPE_UNSIGNED_INT:
                    componentSize = sizeof(uint32_t);
                    break;
                default:
                    return fallbackMaxVertex;
            }

            const uint64_t stride = bufferView.byte_stride ? bufferView.byte_stride : componentSize;
            if (indexAccessor.count == 0)
                return fallbackMaxVertex;

            if (const uint64_t lastOffset = baseOffset + stride * (indexAccessor.count - 1);
                baseOffset >= buffer.data.count || lastOffset + componentSize > buffer.data.count)
                return fallbackMaxVertex;

            u32 maxIndex = 0;
            for (uint64_t i = 0; i < indexAccessor.count; i++)
            {
                const uint8_t* ptr = buffer.data.data + baseOffset + stride * i;
                u32 value = 0;
                switch (indexAccessor.component_type)
                {
                    case TG3_COMPONENT_TYPE_UNSIGNED_BYTE:
                    {
                        uint8_t v = 0;
                        std::memcpy(&v, ptr, sizeof(uint8_t));
                        value = static_cast<u32>(v);
                        break;
                    }
                    case TG3_COMPONENT_TYPE_UNSIGNED_SHORT:
                    {
                        uint16_t v = 0;
                        std::memcpy(&v, ptr, sizeof(uint16_t));
                        value = static_cast<u32>(v);
                        break;
                    }
                    case TG3_COMPONENT_TYPE_UNSIGNED_INT:
                    {
                        uint32_t v = 0;
                        std::memcpy(&v, ptr, sizeof(uint32_t));
                        value = v;
                        break;
                    }
                    default:
                        break;
                }
                maxIndex = std::max(maxIndex, value);
            }

            return maxIndex;
        };

        // nVidia vk_raytracing_tutorial helper func
        auto alignUp = [](auto value, size_t alignment) noexcept { return ((value + alignment - 1) & ~(alignment - 1)); };

        const auto vulkanBuffer = std::dynamic_pointer_cast<Graphics::VulkanGeneralBuffer>(GPUBuffer);
        if (!vulkanBuffer)
        {
            // This should not happen but just in case
            Log<LogLevel::Error>("Provided GPU buffer is not a VulkanGeneralBuffer, which is required for VulkanSceneMesh. Skipping mesh {}.", meshIndex);
            return;
        }

        std::vector<vk::AccelerationStructureGeometryKHR> geometries;
        std::vector<u32> primitiveCount;
        std::vector<vk::AccelerationStructureBuildRangeInfoKHR> buildRanges;
        std::vector<vk::AccelerationStructureGeometryTrianglesDataKHR> triangles;

        geometries.reserve(mesh.primitives_count);
        primitiveCount.reserve(mesh.primitives_count);
        buildRanges.reserve(mesh.primitives_count);
        triangles.reserve(mesh.primitives_count);

        for (u32 i = 0; i < mesh.primitives_count; i++)
        {
            const tg3_primitive& prim = mesh.primitives[i];

            if (prim.mode != TG3_MODE_TRIANGLES && prim.mode != DEFAULT_TRIANGLES)
            {
                Log<LogLevel::Error>("Mesh primitive {} in mesh {} is not in triangle mode, which is required for ray tracing. Skipping this primitive.", i, meshIndex);
                continue;
            }
            if (prim.indices == -1)
            {
                Log<LogLevel::Warn>("Mesh primitive {} in mesh {} does not have an index accessor, which is required for ray tracing. Skipping this primitive.", i, meshIndex);
                continue;
            }

            AttributeData positionData = extractAttribute(prim, "POSITION");

            auto& accessor = model.accessors[prim.indices];
            auto& bufferView = model.buffer_views[accessor.buffer_view];
            AttributeData indexData = {
                .offset = static_cast<u32>(bufferView.byte_offset + accessor.byte_offset),
                .count = static_cast<u32>(accessor.count),
                .byteStride = (bufferView.byte_stride ? static_cast<u32>(bufferView.byte_stride) : 0 /* FIXME: 0 for now */),
                .bufferIndex = static_cast<u32>(bufferView.buffer),
                .hasData = true
            };

            const u32 fallbackMaxVertex = positionData.count > 0 ? positionData.count - 1 : 0;
            const u32 maxIndex = computeMaxIndex(accessor, fallbackMaxVertex);

            const u32 positionStride = positionData.byteStride > 0 ? positionData.byteStride : static_cast<u32>(sizeof(float) * 3);

            triangles.emplace_back(
                vk::Format::eR32G32B32Sfloat,
                vk::DeviceOrHostAddressConstKHR().setDeviceAddress(vulkanBuffer->GetBufferDeviceAddress() + positionData.offset),
                positionStride,
                maxIndex,
                accessor.component_type == TG3_COMPONENT_TYPE_UNSIGNED_SHORT ? vk::IndexType::eUint16 : vk::IndexType::eUint32,
                vk::DeviceOrHostAddressConstKHR().setDeviceAddress(vulkanBuffer->GetBufferDeviceAddress() + indexData.offset)
            );

            geometries.emplace_back(
                vk::GeometryTypeKHR::eTriangles,
                vk::AccelerationStructureGeometryDataKHR()
                    .setTriangles(triangles.back())
            );

            primitiveCount.emplace_back(indexData.count / 3);
            buildRanges.emplace_back(indexData.count / 3, 0, 0, 0);

            auto normalData = extractAttribute(prim, "NORMAL");
            auto colourData = extractAttribute(prim, "COLOR_0");
            assert((normalData.byteStride == 12 || normalData.hasData == false) && (colourData.byteStride == 16 || colourData.hasData == false));
            if (accessor.component_type != TG3_COMPONENT_TYPE_UNSIGNED_SHORT)
                OWCDebugBreak();

            /*auto texCoordsdata = extractAttribute(prim, "TEXCOORD_0");
            auto Tangentsdata = extractAttribute(prim, "TANGENT");*/
            GPUData.emplace_back(
                positionData.offset,
                indexData.offset,
                normalData.hasData ? static_cast<u64>(normalData.offset) : ~0ULL,
                colourData.hasData ? static_cast<u64>(colourData.offset) : ~0ULL,
                prim.material,
                accessor.component_type == TG3_COMPONENT_TYPE_UNSIGNED_SHORT ? static_cast<u32>(true) : static_cast<u32>(false)
            );
        }

        auto buildInfo = vk::AccelerationStructureBuildGeometryInfoKHR()
            .setType(vk::AccelerationStructureTypeKHR::eBottomLevel)
            .setFlags(vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace | vk::BuildAccelerationStructureFlagBitsKHR::eAllowCompaction)
            .setGeometries(geometries);

        Log<LogLevel::Trace>("Mesh {} built {} geometries from {} primitives", meshIndex, geometries.size(), mesh.primitives_count);

        vk::AccelerationStructureBuildSizesInfoKHR buildSizes = device.getAccelerationStructureBuildSizesKHR(
            vk::AccelerationStructureBuildTypeKHR::eDevice,
            buildInfo,
            primitiveCount
        );

        const vk::DeviceSize scratchBufferSize = alignUp(buildSizes.buildScratchSize, vkCore.GetAccelerationStructureProperties().minAccelerationStructureScratchOffsetAlignment);

        VulkanGeneralBuffer scratchBuffer(scratchBufferSize);

        constexpr auto bufferUsageInfo2 = vk::BufferUsageFlags2CreateInfo()
            .setUsage(
                vk::BufferUsageFlagBits2::eShaderDeviceAddress |
                vk::BufferUsageFlagBits2::eAccelerationStructureBuildInputReadOnlyKHR |
                vk::BufferUsageFlagBits2::eAccelerationStructureStorageKHR
            );

        auto bufferInfo = vk::BufferCreateInfo()
            .setPNext(&bufferUsageInfo2)
            .setSize(buildSizes.accelerationStructureSize)
            .setSharingMode(vk::SharingMode::eConcurrent)
            .setQueueFamilyIndices(queueIndices);

        constexpr vma::AllocationCreateInfo allocInfo = vma::AllocationCreateInfo()
            .setUsage(vma::MemoryUsage::eGpuOnly)
            .setFlags(vma::AllocationCreateFlagBits::eDedicatedMemory);

        auto scratchBLASBuffer = vma::raii::Buffer(allocator, bufferInfo, allocInfo);

        auto accelerationStructureCreateInfo = vk::AccelerationStructureCreateInfoKHR()
            .setType(vk::AccelerationStructureTypeKHR::eBottomLevel)
            .setSize(buildSizes.accelerationStructureSize)
            .setBuffer(scratchBLASBuffer);

        auto scratchBLAS = device.createAccelerationStructureKHR(accelerationStructureCreateInfo);

        buildInfo.setScratchData(vk::DeviceOrHostAddressKHR().setDeviceAddress(scratchBuffer.GetBufferDeviceAddress()))
            .setDstAccelerationStructure(scratchBLAS);

        auto cmd = vkCore.GetSingleTimeComputeCommandBuffer();

        vk::raii::QueryPool queryPool(device, vk::QueryPoolCreateInfo()
            .setQueryType(vk::QueryType::eAccelerationStructureCompactedSizeKHR)
            .setQueryCount(1));

        const auto buildRangesPtr = buildRanges.data();
        cmd.begin(vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
        cmd.buildAccelerationStructuresKHR(buildInfo, buildRangesPtr);
        cmd.resetQueryPool(*queryPool, 0, 1);
        cmd.writeAccelerationStructuresPropertiesKHR(*scratchBLAS, vk::QueryType::eAccelerationStructureCompactedSizeKHR, *queryPool, 0);
        cmd.end();

        auto fence = device.createFence(vk::FenceCreateInfo());
        auto cmdSubmitInfo = vk::CommandBufferSubmitInfo().setCommandBuffer(cmd);
        vkCore.GetComputeQueue().submit2(vk::SubmitInfo2().setCommandBufferInfos(cmdSubmitInfo), *fence);
        if(device.waitForFences(*fence, VK_TRUE, std::numeric_limits<uint64_t>::max()) != vk::Result::eSuccess)
            Log<LogLevel::Critical>("Failed to wait for acceleration structure build fence");

        const auto [result, sizes] = (*device).getQueryPoolResults<vk::DeviceSize>(*queryPool, 0, 1, sizeof(vk::DeviceSize), sizeof(vk::DeviceSize));
        if (result != vk::Result::eSuccess)
        {
            Log<LogLevel::Error>("Failed to get compacted size for BLAS of mesh {}, using original size", meshIndex);
            m_Buffer = std::move(scratchBLASBuffer);
            m_AccelerationStructure = std::move(scratchBLAS);
            m_BufferDeviceAddress = vkCore.GetDevice().getAccelerationStructureAddressKHR(vk::AccelerationStructureDeviceAddressInfoKHR().setAccelerationStructure(m_AccelerationStructure));
            return;
        }

        Log<LogLevel::Trace>("Mesh {} BLAS starting size {}, compaction size: {}", meshIndex, scratchBufferSize, sizes[0]);

        bufferInfo.setSize(sizes[0]);
        m_Buffer = vma::raii::Buffer(allocator, bufferInfo, allocInfo);

        accelerationStructureCreateInfo.setSize(sizes[0]).setBuffer(m_Buffer);

        m_AccelerationStructure = device.createAccelerationStructureKHR(accelerationStructureCreateInfo);
        m_BufferDeviceAddress = vkCore.GetDevice().getAccelerationStructureAddressKHR(vk::AccelerationStructureDeviceAddressInfoKHR().setAccelerationStructure(m_AccelerationStructure));

        const auto copyAccelerationStructureInfo = vk::CopyAccelerationStructureInfoKHR()
            .setSrc(scratchBLAS)
            .setDst(m_AccelerationStructure)
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

    /*const AttributeData& VulkanSceneMesh::GetAttributeData(const std::string& attributeName) const
    {
        static AttributeData emptyData{};
        return emptyData;
    }*/
} // OWC