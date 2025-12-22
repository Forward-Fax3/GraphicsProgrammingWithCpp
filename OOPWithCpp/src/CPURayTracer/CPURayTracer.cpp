#include "Core.hpp"
#include "Application.hpp"
#include "CPURayTracer.hpp"
#include "ImageLoader.hpp"

#include "BaseEvent.hpp"
#include "WindowResize.hpp"

#include <glm/gtc/type_ptr.hpp>

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
			m_InterLayerData->imageScreenSize = Vec2u(0);
			m_InterLayerData->ImageUpdates |= 0b10;
			return;
		}

		if (!m_ToggleRaytracedImage)
			return;

		if (m_ToggleRaytracedImage && m_RayTracingStateUpdated)
		{
			m_InterLayerData->imageData.clear();
			m_InterLayerData->imageScreenSize = Application::GetConstInstance().GetWindowSize();
			m_InterLayerData->imageData.resize(m_InterLayerData->GetNumberOfPixels<uSize>());
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

			if (ImGui::Combo("Scene", &m_CurrentSceneIndex, sceneNames.data(), static_cast<i32>(sceneNames.size())))
			{
				auto selectedScene = static_cast<Scene>(m_CurrentSceneIndex);
				m_Scene = BaseScene::CreateScene(selectedScene, m_InterLayerData->imageData);
				m_InterLayerData->numberOfSamples = 0;
				m_InterLayerData->ImageUpdates |= 0b01;
				for (auto& pixel : m_InterLayerData->imageData)
					pixel = Vec4(0.0f);
			}

			if (ImGui::Combo("Gamma Correction", &m_CurrentGammaIndex, gammaCorrectionNames.data(), static_cast<i32>(gammaCorrectionNames.size())))
			{
				auto gamma = static_cast<GammaCorrection>(m_CurrentGammaIndex);
				if (gamma != GammaCorrection::custom)
					UpdateGammaValue(gamma);
				else 
					m_InterLayerData->invGammaValue = 1.0f / m_CustomGammaValue;
			}
			if (static_cast<GammaCorrection>(m_CurrentGammaIndex) == GammaCorrection::custom && ImGui::InputFloat("Custom Gamma Value", &m_CustomGammaValue))
				m_InterLayerData->invGammaValue = 1.0f / m_CustomGammaValue;

			bool useCustomResolutionUpdated = ImGui::Checkbox("Use Window Resolution", &m_UseWindowResolution);

			if (!m_UseWindowResolution)
			{
				Vec2i customResolution(m_InterLayerData->imageScreenSize);
				useCustomResolutionUpdated |= ImGui::InputInt2("Image Resolution", glm::value_ptr(customResolution));


				if (useCustomResolutionUpdated)
				{
					customResolution = glm::max(customResolution, Vec2i(1));
					m_InterLayerData->imageScreenSize = customResolution;
					m_InterLayerData->imageData.clear();
					m_InterLayerData->imageData.resize(m_InterLayerData->GetNumberOfPixels<uSize>());
					m_InterLayerData->numberOfSamples = 0;
					m_InterLayerData->ImageUpdates |= 0b10;
					m_Scene->UpdateScreenSize(m_InterLayerData->imageScreenSize);
				}
			}
			else if (useCustomResolutionUpdated)
			{
				m_InterLayerData->imageScreenSize = Application::GetConstInstance().GetWindowSize();
				m_InterLayerData->imageData.clear();
				m_InterLayerData->imageData.resize(m_InterLayerData->GetNumberOfPixels<uSize>());
				m_InterLayerData->numberOfSamples = 0;
				m_InterLayerData->ImageUpdates |= 0b10;
				m_Scene->UpdateScreenSize(m_InterLayerData->imageScreenSize);
			}
		}
		ImGui::End();

		if (m_ToggleRaytracedImage)
			m_Scene->OnImGuiRender();
	}

	void CPURayTracer::OnEvent(BaseEvent& e)
	{
		EventDispatcher dispatcher(e);

		dispatcher.Dispatch<WindowResize>([this](const WindowResize&) {
			if (!this->m_ToggleRaytracedImage || !this->m_UseWindowResolution)
				return false;

			std::vector<Vec4>& pixelArray = this->m_InterLayerData->imageData;
			pixelArray.clear();
			this->m_InterLayerData->imageScreenSize = Application::GetConstInstance().GetWindowSize();
			pixelArray.resize(this->m_InterLayerData->GetNumberOfPixels<uSize>());
			this->m_InterLayerData->numberOfSamples = 0;
			this->m_InterLayerData->ImageUpdates |= 0b10;
			this->m_Scene->UpdateScreenSize(this->m_InterLayerData->imageScreenSize);
			return false;
		});
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
		default:
			std::unreachable();
		}
	}
}