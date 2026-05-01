//
// Created by forwardfax3 on 17/04/2026.
//

#include "BaseTLAS.hpp"
#include "VulkanTLAS.hpp"


namespace OWC
{
    std::shared_ptr<BaseTLAS> BaseTLAS::CreateTopLevelAccelerationStructure()
    {
        // Only Vulkan Supported
        return std::make_shared<VulkanTLAS>();
    }
}
