#include "Application.hpp"
#include "MainLayer.hpp"
#include "RenderLayer.hpp"
#include "CPURayTracer.hpp"
#include "InterLayerData.hpp"

#include <memory>


namespace OWC
{
	MainLayer::MainLayer()
	{
		Application& app = Application::GetInstance();
		auto ILD = std::make_shared<InterLayerData>();
		m_TestLayer = std::make_shared<RenderLayer>(ILD);
		m_CPURayTracerLayer = std::make_shared<CPURayTracer>(ILD);
		app.PushLayer(m_TestLayer);
		app.PushLayer(m_CPURayTracerLayer);
	}
}