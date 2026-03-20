#include "Application.hpp"
#include "MainLayer.hpp"
#include "CPURayTracerRenderer.hpp"
#include "CPURayTracer.hpp"
#include "InterLayerData.hpp"

#include <memory>


namespace OWC
{
	MainLayer::MainLayer()
	{
		Application& app = Application::GetInstance();
		auto ILD = std::make_shared<InterLayerData>();
		m_TestLayer = std::make_shared<CPURayTracerRenderer>(ILD);
		m_CPURayTracerLayer = std::make_shared<CPURayTracer>(ILD);
		m_GPURayTracerLayer = std::make_shared<GPURayTracerRenderer>();
		app.PushLayer(m_TestLayer);
		app.PushLayer(m_CPURayTracerLayer);
		app.PushLayer(m_GPURayTracerLayer);

		// m_TestLayer->SetActiveState(false);
		// m_CPURayTracerLayer->SetActiveState(false);
		m_GPURayTracerLayer->SetActiveState(false);
	}
}