#include "core.hpp"
#include "Application.hpp"
#include "CPURayTracer.hpp"
#include "ImageLoader.hpp"

#include "BaseEvent.hpp"
#include "WindowResize.hpp"

#include <array>


namespace OWC
{
	CPURayTracer::CPURayTracer(const std::shared_ptr<InterLayerData>& interLayerData)
		: m_InterLayerData(interLayerData)
	{
		m_Scene = BaseScene::CreateScene(Scene::Basic, m_InterLayerData->imageData);
		m_InterLayerData->numberOfSamples = 1;
	}

	void CPURayTracer::OnUpdate()
	{
		if (!m_ToggleRaytracedImage && m_RayTracingStateUpdated)
		{
			m_InterLayerData->imageData.clear();
			m_InterLayerData->imageHeight = 0;
			m_InterLayerData->imageWidth = 0;
			m_InterLayerData->ImageUpdates |= 0b10;
			return;
		}

		if (!m_ToggleRaytracedImage)
			return;

		if (m_ToggleRaytracedImage && m_RayTracingStateUpdated)
		{
			m_InterLayerData->imageData.clear();
			m_InterLayerData->imageHeight = Application::GetConstInstance().GetWindowHeight();
			m_InterLayerData->imageWidth = Application::GetConstInstance().GetWindowWidth();
			m_InterLayerData->imageData.resize(m_InterLayerData->imageWidth * m_InterLayerData->imageHeight);
			m_InterLayerData->numberOfSamples = 0;
			m_InterLayerData->ImageUpdates |= 0b10;
		}

		if (RenderPassReturnData updateImage = m_Scene->RenderNextPass(); updateImage.frameBufferUpdated)
		{
			if (updateImage.frameNeedsReset)
			{
				for (auto& pixel : m_InterLayerData->imageData)
					pixel = Vec4(0.0f);
				m_InterLayerData->numberOfSamples = 0;
			}
			else
			{
				m_InterLayerData->numberOfSamples++;
			}
			m_InterLayerData->ImageUpdates |= 0b01;
		}
	}

	void CPURayTracer::ImGuiRender()
	{
		constexpr std::array<const char*, 7> gammaCorrectionNames = {
			"None",
			"Gamma 1.9",
			"Gamma 2.0",
			"Gamma 2.2",
			"Gamma 2.4",
			"BT. 1886",
			"Custom"
		};
		constexpr std::array<const char*, 3> sceneNames = {
			"Basic",
			"RandTest", 
			"DuelGraySpheres"
		};

		ImGui::Begin("CPU Ray Tracer");
		ImGui::Text("CPU Ray Tracer Layer");
		m_RayTracingStateUpdated = ImGui::Checkbox("Toggle RayTracing", &m_ToggleRaytracedImage);
		if (m_ToggleRaytracedImage)
		{
			ImGui::Text("number of samples %s", std::format("{}", m_InterLayerData->numberOfSamples).c_str());

			if (ImGui::Combo("Scene", &m_CurrentSceneIndex, sceneNames.data(), static_cast<int>(sceneNames.size())))
			{
				auto selectedScene = static_cast<Scene>(m_CurrentSceneIndex);
				m_Scene = BaseScene::CreateScene(selectedScene, m_InterLayerData->imageData);
				m_InterLayerData->numberOfSamples = 0;
				m_InterLayerData->ImageUpdates |= 0b01;
				for (auto& pixel : m_InterLayerData->imageData)
					pixel = Vec4(0.0f);
			}

			if (ImGui::Combo("Gamma Correction", &m_CurrentGammaIndex, gammaCorrectionNames.data(), static_cast<int>(gammaCorrectionNames.size())))
			{
				auto gamma = static_cast<GammaCorrection>(m_CurrentGammaIndex);
				if (gamma != GammaCorrection::custom)
					UpdateGammaValue(gamma);
			}
			if (static_cast<GammaCorrection>(m_CurrentGammaIndex) == GammaCorrection::custom && ImGui::InputFloat("Custom Gamma Value", &m_CustomGammaValue))
				m_InterLayerData->invGammaValue = 1.0f / m_CustomGammaValue;
		}
		ImGui::End();

		if (m_ToggleRaytracedImage)
			m_Scene->OnImGuiRender();
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

		if (m_ToggleRaytracedImage)
			m_Scene->OnEvent(e);
	}

	void CPURayTracer::UpdateGammaValue(GammaCorrection gammaCorrection) const
	{
		switch (gammaCorrection)
		{
		case GammaCorrection::None:
			m_InterLayerData->invGammaValue = 1.0f;
			break;
		case GammaCorrection::Gamma19:
			m_InterLayerData->invGammaValue = 1.0f / 1.9f;
			break;
		case GammaCorrection::Gamma20:
			m_InterLayerData->invGammaValue = 1.0f / 2.0f;
			break;
		case GammaCorrection::Gamma22:
			m_InterLayerData->invGammaValue = 1.0f / 2.2f;
			break;
		case GammaCorrection::Gamma24:
		case GammaCorrection::GammaBT1886:
			m_InterLayerData->invGammaValue = 1.0f / 2.4f;
			break;
		case GammaCorrection::custom:
			// Keep current value
			break;
		default:
			std::unreachable();
		}
	}
}