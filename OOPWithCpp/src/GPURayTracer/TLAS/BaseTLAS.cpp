//
// Created by forwardfax3 on 17/04/2026.
//

#include "BaseTLAS.hpp"
#include "VulkanTLAS.hpp"


namespace OWC
{
    std::shared_ptr<BaseTLAS> BaseTLAS::CreateTopLevelAccelerationStructure(const std::map<i32, std::unique_ptr<SceneMesh>>& meshes, const std::vector<std::pair<Mat4, i32>>& meshIndices)
    {
        // Only Vulkan Supported
        return std::make_shared<VulkanTLAS>(meshes, meshIndices);
    }
}
