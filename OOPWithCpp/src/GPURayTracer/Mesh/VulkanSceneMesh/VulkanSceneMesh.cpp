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

#define DEFAULT_TRIANGLES static_cast<u32>(-1)


namespace OWC
{
    VulkanSceneMesh::VulkanSceneMesh(const tg3_model& model, const i32 meshIndex, i32 customInstancesIndex, const std::shared_ptr<Graphics::GeneralBuffer>& GPUBuffer, std::vector<GPUGLTFData>& GPUData)
        : m_Model(model), m_CustomInstanceIndex(customInstancesIndex)
    {
        using namespace OWC::Graphics;
		const auto& vkCore = VulkanCore::GetConstInstance();
        const auto& device = vkCore.GetDevice();

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

        geometries.reserve(mesh.primitives_count);
        primitiveCount.reserve(mesh.primitives_count);
        buildRanges.reserve(mesh.primitives_count);

        {
            std::vector<vk::AccelerationStructureGeometryTrianglesDataKHR> triangles;
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

                primitiveCount.emplace_back(positionData.count / 3);
                buildRanges.emplace_back(positionData.count / 3, positionData.offset, 0, 0);

                triangles.emplace_back(
                    vk::Format::eR32G32B32Sfloat,
                    vk::DeviceOrHostAddressConstKHR().setDeviceAddress(vulkanBuffer->GetBufferDeviceAddress() + positionData.offset),
                    positionData.byteStride,
                    positionData.count,
                    accessor.component_type == TG3_COMPONENT_TYPE_UNSIGNED_SHORT ? vk::IndexType::eUint16 : vk::IndexType::eUint32,
                    vk::DeviceOrHostAddressConstKHR().setDeviceAddress(vulkanBuffer->GetBufferDeviceAddress() + indexData.offset)
                );

                auto normalData = extractAttribute(prim, "NORMAL");
                auto colourData = extractAttribute(prim, "COLOR_0");
                /*
                auto texCoordsdata = extractAttribute(prim, "TEXCOORD_0");
                auto Tangentsdata = extractAttribute(prim, "TANGENT");*/
                GPUData.emplace_back(
                    positionData.offset,
                    indexData.offset,
                    normalData.hasData ? normalData.offset : ~0,
                    colourData.hasData ? colourData.offset : ~0,
                    prim.material
                );
            }
        }

        auto buildInfo = vk::AccelerationStructureBuildGeometryInfoKHR()
            .setType(vk::AccelerationStructureTypeKHR::eBottomLevel)
            .setFlags(vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace)
            .setGeometries(geometries);

        vk::AccelerationStructureBuildSizesInfoKHR buildSizes;
        device.getAccelerationStructureBuildSizesKHR(
            vk::AccelerationStructureBuildTypeKHR::eDevice,
            &buildInfo,
            primitiveCount.data(),
            &buildSizes
        );

        const vk::DeviceSize scratchBufferSize = alignUp(buildSizes.buildScratchSize, vkCore.GetAccelerationStructureProperties().minAccelerationStructureScratchOffsetAlignment);

        VulkanGeneralBuffer scratchBuffer(scratchBufferSize);
        m_AccelerationStructureBuffer = std::make_shared<VulkanGeneralBuffer>(buildSizes.accelerationStructureSize);

        const auto accelerationStructureCreateInfo = vk::AccelerationStructureCreateInfoKHR()
            .setType(vk::AccelerationStructureTypeKHR::eBottomLevel)
            .setSize(buildSizes.accelerationStructureSize)
            .setBuffer(m_AccelerationStructureBuffer->GetBuffer());

        m_AccelerationStructure = device.createAccelerationStructureKHR(accelerationStructureCreateInfo);

        buildInfo.setScratchData(vk::DeviceOrHostAddressKHR().setDeviceAddress(scratchBuffer.GetBufferDeviceAddress()))
            .setDstAccelerationStructure(m_AccelerationStructure);

        auto cmd = vkCore.GetSingleTimeComputeCommandBuffer();

        const auto buildRangesPtr = buildRanges.data();
        cmd.begin(vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
        cmd.buildAccelerationStructuresKHR(1, &buildInfo, &buildRangesPtr);
        cmd.end();

        auto fence = device.createFence(vk::FenceCreateInfo());
        vkCore.GetComputeQueue().submit(vk::SubmitInfo().setCommandBuffers(cmd), fence);
        if(device.waitForFences(fence, VK_TRUE, std::numeric_limits<uint64_t>::max()) != vk::Result::eSuccess)
            Log<LogLevel::Critical>("Failed to wait for acceleration structure build fence");
        device.destroyFence(fence);
    }

    VulkanSceneMesh::~VulkanSceneMesh()
    {
         if (m_AccelerationStructure)
         {
             const auto& vkCore = Graphics::VulkanCore::GetConstInstance();
             vkCore.GetDevice().destroyAccelerationStructureKHR(m_AccelerationStructure);
         }
    }

    const AttributeData& VulkanSceneMesh::GetAttributeData(const std::string& attributeName) const
    {
        static AttributeData emptyData{};
        return emptyData;
    }
} // OWC