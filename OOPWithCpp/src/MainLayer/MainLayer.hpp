#pragma once
#include "Layer.hpp"
#include "CPURayTracerRenderer.hpp"
#include "CPURayTracer.hpp"
#include "GPURayTracerRenderer.hpp"


namespace OWC
{
	class MainLayer : public Layer
	{
	public:
		MainLayer();
		~MainLayer() override = default;
		MainLayer(const MainLayer&) = delete;
		MainLayer& operator=(const MainLayer&) = delete;
		MainLayer(MainLayer&&) = delete;
		MainLayer&& operator=(MainLayer&&) = delete;

		void OnUpdate() override;
		void ImGuiRender() override;

	private:
		std::shared_ptr<CPURayTracerRenderer> m_TestLayer;
		std::shared_ptr<CPURayTracer> m_CPURayTracerLayer;
		std::shared_ptr<GPURayTracerRenderer> m_GPURayTracerLayer;

		std::shared_ptr<Graphics::RenderPassData> m_EmptyRenderPass; // used to use the semaphore that needs waiting on

		u8 m_ActiveLayer = 0; // 1 for CPU Ray Tracer, 2 for GPU Ray Tracer
	};
}
