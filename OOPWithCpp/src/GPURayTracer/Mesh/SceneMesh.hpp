//
// Created by forwardfax3 on 15/04/2026.
//

#pragma once
#include "Core.hpp"
#include "UniformBuffer.hpp"

#include <tiny_gltf_v3.h>
#include <memory>


namespace OWC
{
    struct GPUGLTFData
    {
        u64 positionsOffset;
        u64 indicesOffset;
        u64 normalsOffset;
        u64 coloursOffset;
        u32 materialIndex;
        u32 _[3]; // pad to 48 bytes
    };

    struct AttributeData
    {
        u32 offset = ~0;
        u32 count = ~0;
        u32 byteStride = ~0;
        u32 bufferIndex = ~0;
        bool hasData = false;
    };

    class SceneMesh
    {
    public:
        SceneMesh() = default;
        virtual ~SceneMesh() = default;
        SceneMesh(SceneMesh&) = delete;
        SceneMesh& operator=(SceneMesh&) = delete;
        SceneMesh(SceneMesh&&) noexcept = delete;
        SceneMesh& operator=(SceneMesh&&) noexcept = delete;

        [[nodiscard]] virtual const AttributeData& GetAttributeData(const std::string& attributeName) const = 0;

        static std::shared_ptr<SceneMesh> CreateFromGLTFModelWithMeshIndex(const tg3_model& gltfMesh, i32 meshIndex, i32 customInstancesIndex, const std::shared_ptr<Graphics::GeneralBuffer>& GPUBuffer, std::vector<GPUGLTFData>& GPUData);
    };
} // OWC
