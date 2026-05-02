//
// Created by forwardfax3 on 17/04/2026.
//

#pragma once
#include <memory>
#include <map>

#include "Scene/BaseGPUScene.hpp"
#include "SceneMesh.hpp"
#include "BaseTLAS.hpp"
#include "UniformBuffer.hpp"

namespace OWC
{
    class SponzaPalace : public BaseGPUScene
    {
    public:
        SponzaPalace();
        ~SponzaPalace() override;
        SponzaPalace(SponzaPalace&) = delete;
        SponzaPalace& operator=(SponzaPalace&) = delete;
        SponzaPalace(SponzaPalace&&) noexcept = delete;
        SponzaPalace& operator=(SponzaPalace&&) noexcept = delete;

    private:
        void IterateThroughNodes(const tg3_model& model, u32 nodeIndex, Mat4 parentTransform);

    private:
        std::map<i32, std::shared_ptr<SceneMesh>> m_Meshes;
        tg3_model m_Model = {};
        std::shared_ptr<BaseTLAS> m_TLAS;
        std::shared_ptr<Graphics::GeneralBuffer> m_GPUBuffer;
    };
}// OWC
