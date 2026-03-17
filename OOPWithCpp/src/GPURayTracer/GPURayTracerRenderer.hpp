//
// Created by forwardfax3 on 15/03/2026.
//
#pragma once
#include <memory>

#include "Layer.hpp"
#include "BaseShader.hpp"
#include "Renderer.hpp"
#include "UniformBuffer.hpp"
#include "InterLayerData.hpp"


namespace OWC
{
    class GPURayTracerRenderer : public Layer
    {
    private: // structs
        struct UniformBufferObject
        {
            f32 divider = 0.0f;
            f32 invGammaValue = 0.0f;
        };

    public:
        GPURayTracerRenderer();
        ~GPURayTracerRenderer() override = default;
        GPURayTracerRenderer(const GPURayTracerRenderer&) = delete;
        GPURayTracerRenderer& operator=(const GPURayTracerRenderer&) = delete;
        GPURayTracerRenderer(GPURayTracerRenderer&&) = delete;
        GPURayTracerRenderer& operator=(GPURayTracerRenderer&&) = delete;

        void OnUpdate() override;
        void OnEvent(class BaseEvent&) override;

    private: // methods
        void SetupRenderPass();
        void SetupPipeline();

    private: // attributes
        std::unique_ptr<Graphics::BaseShader> m_Shader = nullptr;
        std::shared_ptr<Graphics::RenderPassData> m_renderPass = nullptr;
        std::shared_ptr<Graphics::UniformBuffer> m_UniformBuffer = nullptr;
        std::shared_ptr<Graphics::DynamicTextureBuffer> m_Image = nullptr;
        std::shared_ptr<InterLayerData> m_ILD = nullptr;
        //uSize framesUpdated = 0;
    };
} // OWC
