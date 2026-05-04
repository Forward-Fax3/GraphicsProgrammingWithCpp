//
// Created by forwardfax3 on 16/04/2026.
//

#pragma once
#include "BaseTLAS.hpp"


namespace OWC
{
    class BaseGPUScene
    {
    public:
        BaseGPUScene() = default;
        virtual ~BaseGPUScene() = default;
        BaseGPUScene(BaseGPUScene&) = delete;
        BaseGPUScene& operator=(BaseGPUScene&) = delete;
        BaseGPUScene(BaseGPUScene&&) noexcept = delete;
        BaseGPUScene& operator=(BaseGPUScene&&) noexcept = delete;

        virtual std::shared_ptr<BaseTLAS>& GetTLAS() = 0;
        virtual uSize GetDeviceMegaBufferPtr() const = 0;
        virtual uSize GetDeviceGeometryBufferPtr() const = 0;
    };
}
