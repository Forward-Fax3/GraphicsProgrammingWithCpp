//
// Created by forwardfax3 on 17/04/2026.
//

#pragma once
#include <memory>

#include "Core.hpp"
#include "SceneMesh.hpp"


namespace OWC
{
    class BaseTLAS
    {
    public:
        BaseTLAS() = default;
        virtual ~BaseTLAS() = default;
        BaseTLAS(BaseTLAS&) = delete;
        BaseTLAS& operator=(BaseTLAS&) = delete;
        BaseTLAS(BaseTLAS&&) noexcept = delete;
        BaseTLAS& operator=(BaseTLAS&&) noexcept = delete;

        virtual void AddInstance(const Mat4& transform, const std::shared_ptr<SceneMesh>& mesh) = 0;
        virtual void CreateTLAS() = 0;

        static std::shared_ptr<BaseTLAS> CreateTopLevelAccelerationStructure();
    };
}
