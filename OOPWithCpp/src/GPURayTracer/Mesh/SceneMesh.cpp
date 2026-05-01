//
// Created by forwardfax3 on 15/04/2026.
//

#include "SceneMesh.hpp"
#include "VulkanSceneMesh.hpp"


namespace OWC
{
    std::shared_ptr<SceneMesh> SceneMesh::CreateFromGLTFModelWithMeshIndex(const tg3_model& gltfMesh, i32 meshIndex, const std::shared_ptr<Graphics::GeneralBuffer>& GPUBuffer)
    {
        // only vulkan supported at the moment
        return std::make_shared<VulkanSceneMesh>(gltfMesh, meshIndex, GPUBuffer);
    }
} // OWC