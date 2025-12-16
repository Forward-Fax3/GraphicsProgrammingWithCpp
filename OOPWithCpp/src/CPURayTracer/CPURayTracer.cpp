#include "core.hpp"
#include "Application.hpp"
#include "CPURayTracer.hpp"
#include "ImageLoader.hpp"

#include "BaseEvent.hpp"
#include "WindowResize.hpp"


namespace OWC
{
	CPURayTracer::CPURayTracer(const std::shared_ptr<InterLayerData>& interLayerData)
		: m_InterLayerData(interLayerData)
	{
		m_ImageLoader = std::make_unique<ImageLoader>("../cat.jpg");
		m_SecondImageLoader = std::make_unique<ImageLoader>("../cat2.jpg");
		m_Scene = BaseScene::CreateScene(Scene::Basic, m_InterLayerData->imageData);
		m_InterLayerData->numberOfSamples = 1;
	}

	void CPURayTracer::OnUpdate()
	{
		if (m_ShowRaytracedImage && !m_ToggleRaytracedImage)
		{
			m_ImageStateUpdated = true;
		}
		else if (m_ShowRaytracedImage)
		{
			m_InterLayerData->imageData.clear();
			m_InterLayerData->imageHeight = Application::GetConstInstance().GetWindowHeight();
			m_InterLayerData->imageWidth = Application::GetConstInstance().GetWindowWidth();
			m_InterLayerData->imageData.resize(m_InterLayerData->imageWidth * m_InterLayerData->imageHeight);
			m_InterLayerData->numberOfSamples = 0;
			m_InterLayerData->ImageUpdates |= 0b10;
		}

		if (m_ToggleRaytracedImage)
		{
			// perform ray tracing and update m_InterLayerData->imageData accordingly
			// (ray tracing logic not implemented in this snippet)
			m_Scene->RenderNextPass();
			m_InterLayerData->numberOfSamples++;
			m_InterLayerData->ImageUpdates |= 0b01;
		}

		if (!(m_ImageStateUpdated || m_SecondImageStateUpdated))
			return;

		if (m_ToggleImage)
		{
			if (!m_ToggleSecondImage)
			{
				m_InterLayerData->imageData = m_ImageLoader->GetImageData();
				m_InterLayerData->imageHeight = static_cast<uint32_t>(m_ImageLoader->GetHeight());
				m_InterLayerData->imageWidth = static_cast<uint32_t>(m_ImageLoader->GetWidth());
				m_InterLayerData->numberOfSamples = 1;
			}
			else if (m_ToggleSecondImage)
			{
				m_InterLayerData->imageData = m_SecondImageLoader->GetImageData();
				m_InterLayerData->imageHeight = static_cast<uint32_t>(m_SecondImageLoader->GetHeight());
				m_InterLayerData->imageWidth = static_cast<uint32_t>(m_SecondImageLoader->GetWidth());
				m_InterLayerData->numberOfSamples = 1;
			}

			m_InterLayerData->ImageUpdates |= 0b01;

			if (m_ImageStateUpdated)
				m_InterLayerData->ImageUpdates |= 0b10;
		}
		else
		{
			m_InterLayerData->imageData.clear();
			m_InterLayerData->imageHeight = 0;
			m_InterLayerData->imageWidth = 0;
			m_InterLayerData->ImageUpdates |= 0b10;
		}

		m_ImageStateUpdated = false;
		m_SecondImageStateUpdated = false;
	}

	void CPURayTracer::ImGuiRender()
	{
		ImGui::Begin("CPU Ray Tracer");
		ImGui::Text("CPU Ray Tracer Layer");
		m_ImageStateUpdated = ImGui::Checkbox("toggle image", &m_ToggleImage);
		if (m_ToggleImage)
		{
			m_SecondImageStateUpdated = ImGui::Checkbox("toggle second image", &m_ToggleSecondImage);
			m_ShowRaytracedImage = ImGui::Checkbox("show raytraced image", &m_ToggleRaytracedImage);

			if (m_SecondImageStateUpdated && m_ToggleRaytracedImage)
			{
				m_ToggleRaytracedImage = false;
				m_ImageStateUpdated = true;
			}
			if (m_ShowRaytracedImage && m_ToggleSecondImage)
				m_ToggleSecondImage = false;

			if (m_ToggleRaytracedImage)
				ImGui::Text("number of samples %s", std::format("{}", m_InterLayerData->numberOfSamples).c_str());
		}
		ImGui::End();
	}

	void CPURayTracer::OnEvent(class BaseEvent& e)
	{
		EventDispatcher dispatcher(e);

		dispatcher.Dispatch<WindowResize>([this](const WindowResize&) {
			if (!this->m_ToggleRaytracedImage)
				return false;

			std::vector<Vec4>& pixelArray = this->m_InterLayerData->imageData;
			pixelArray.clear();
			this->m_InterLayerData->imageHeight = Application::GetConstInstance().GetWindowHeight();
			this->m_InterLayerData->imageWidth = Application::GetConstInstance().GetWindowWidth();
			pixelArray.resize(this->m_InterLayerData->imageWidth * this->m_InterLayerData->imageHeight);
			this->m_InterLayerData->numberOfSamples = 0;
			this->m_InterLayerData->ImageUpdates |= 0b10;
			return false;
		});
	}
}