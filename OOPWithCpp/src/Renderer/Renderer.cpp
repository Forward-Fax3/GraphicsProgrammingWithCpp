#include "Application.hpp"
#include "Renderer.hpp"


namespace OWC
{
	std::shared_ptr<Graphics::GraphicsContext> Renderer::s_GC = nullptr;

	void Renderer::Init()
	{
		s_GC = Application::GetInstance().GetWindow().GetGraphicsContextPtr().lock();
	}

	void Renderer::Shutdown()
	{
		s_GC.reset();
	}

	void Renderer::FinishRender()
	{
		s_GC->FinishRender();
	}
};