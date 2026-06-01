#include "Application.hpp"
#include "MainLayer.hpp"
#include "CPURayTracerRenderer.hpp"
#include "CPURayTracer.hpp"
#include "InterLayerData.hpp"
#include "WindowMinimizeEvent.hpp"
#include "WindowRestoreEvent.hpp"

#include <array>
#include <string_view>
#include <memory>


namespace OWC
{
	MainLayer::MainLayer()
	{
		m_EmptyRenderPass = Graphics::Renderer::GetStaticRenderPass();
		Graphics::Renderer::EndPass(m_EmptyRenderPass);
	}

	void MainLayer::OnUpdate()
	{
		if (m_ActiveLayer == 0 && !m_IsMinimized)
		{
			std::array<std::string_view, 1> waitSemaphores = { "ImageReady" };
			Graphics::Renderer::SubmitRenderPass(m_EmptyRenderPass, waitSemaphores);
		}
	}

	void MainLayer::ImGuiRender()
	{
		Application& app = Application::GetInstance();

		bool CPURayTracerActive = m_ActiveLayer == 1;
		bool GPURayTracer = m_ActiveLayer == 2;

		ImGui::Begin("mainLayer");

		if (!GPURayTracer)
		{
			if (ImGui::Checkbox("CPU Ray Tracer", &CPURayTracerActive))
			{
				if (CPURayTracerActive)
				{
					auto ILD = std::make_shared<InterLayerData>();
					m_TestLayer = std::make_shared<CPURayTracerRenderer>(ILD);
					m_CPURayTracerLayer = std::make_shared<CPURayTracer>(ILD);

					app.PushLayer(m_TestLayer);
					app.PushLayer(m_CPURayTracerLayer);
					m_ActiveLayer = 1;
				}
				else
				{
					Graphics::Renderer::AddToEndOfFrameCleanUp([testLayer = std::move(m_TestLayer), cpuRayTracerLayer = std::move(m_CPURayTracerLayer), &app]()
						{
							app.PopLayer(testLayer);
							app.PopLayer(cpuRayTracerLayer);
						}
					);
					m_ActiveLayer = 0;
				}
			}
		}
		if (!CPURayTracerActive)
		{
			if (ImGui::Checkbox("GPU Ray Tracer", &GPURayTracer))
			{
				if (GPURayTracer)
				{
					m_GPURayTracerLayer = std::make_shared<GPURayTracerRenderer>();
					app.PushLayer(m_GPURayTracerLayer);
					m_ActiveLayer = 2;
				}
				else
				{
					Graphics::Renderer::AddToEndOfFrameCleanUp([gpuRayTracerLayer = std::move(m_GPURayTracerLayer), &app]()
						{
							app.PopLayer(gpuRayTracerLayer);
						}
					);
					m_ActiveLayer = 0;
				}
			}
		}
		ImGui::End();
	}

	void MainLayer::OnEvent(BaseEvent& event)
	{
		EventDispatcher dispatcher(event);

		dispatcher.Dispatch<WindowRestore>([this](const WindowRestore&)
			{
				m_IsMinimized = false;
				return false;
			});

		if (m_IsMinimized)
			return;

		dispatcher.Dispatch<WindowMinimize>([this](const WindowMinimize&)
			{
				m_IsMinimized = true;
				return false;
			});
	}


}
