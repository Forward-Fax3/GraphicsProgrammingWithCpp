#pragma once
#include "Layer.hpp"
#include "RenderLayer.hpp"
#include "CPURayTracer.hpp"


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
		std::shared_ptr<RenderLayer> m_TestLayer;
		std::shared_ptr<CPURayTracer> m_CPURayTracerLayer;
	};
}