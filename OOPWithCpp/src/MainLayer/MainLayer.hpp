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

	private:
		std::shared_ptr<CPURayTracerRenderer> m_TestLayer;
		std::shared_ptr<CPURayTracer> m_CPURayTracerLayer;
		std::shared_ptr<GPURayTracerRenderer> m_GPURayTracerLayer;
	};
}
