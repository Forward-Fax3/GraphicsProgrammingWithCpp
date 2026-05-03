//
// Created by forwardfax3 on 15/04/2026.
//

#include "SceneMesh.hpp"
#include "VulkanSceneMesh.hpp"


namespace OWC
{
    std::shared_ptr<SceneMesh> SceneMesh::CreateFromGLTFModelWithMeshIndex(const tg3_model& gltfMesh, i32 meshIndex, i32 customInstancesIndex, const std::shared_ptr<Graphics::GeneralBuffer>& GPUBuffer, std::vector<GPUGLTFData>& GPUData)
    {
        // only vulkan supported at the moment
        return std::make_shared<VulkanSceneMesh>(gltfMesh, meshIndex, customInstancesIndex, GPUBuffer, GPUData);
    }
} // OWC