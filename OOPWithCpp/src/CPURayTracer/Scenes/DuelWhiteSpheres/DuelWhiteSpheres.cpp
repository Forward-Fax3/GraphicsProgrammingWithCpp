#include "Core.hpp"
#include "Application.hpp"
#include "DuelWhiteSpheres.hpp"

#include "Hittables.hpp"

#include "Sphere.hpp"
#include "Lambertian.hpp"

#include "BaseEvent.hpp"
#include "WindowResize.hpp"

#include <imgui.h>

#include <glm/gtc/type_ptr.hpp>


namespace OWC
{
	DuelGraySpheres::DuelGraySpheres(std::vector<glm::vec4>& frameBuffer)
		: BaseScene(frameBuffer)
	{
		m_SceneObjects = std::make_shared<Hitables>();
		m_SceneObjects->Reserve(2);

		auto gray = std::make_shared<Lambertian>(Colour(0.5f));
		auto smallSphere = std::make_shared<Sphere>(glm::vec3(0.0f), 1.0f, gray);
		auto bigSphere = std::make_shared<Sphere>(glm::vec3(0.0f, 51.0f, 0.0f), 50.0f, gray);
		m_SceneObjects->AddObject(smallSphere);
		m_SceneObjects->AddObject(bigSphere);

		m_Camera = std::make_unique<RTCamera>(m_SceneObjects, GetFrameBuffer());
		m_Camera->SetPosition(m_CameraPosition);
		m_Camera->SetRotate(m_CameraRotation);
		m_Camera->SetFOV(m_FOV);
		m_Camera->SetFocalLength(m_FocalLength);
	}

	OWC::RenderPassReturnData DuelGraySpheres::RenderNextPass()
	{
		static bool imageWasClearedLastPass = false;

		if (m_ImageNeedsClearing && imageWasClearedLastPass)
		{
			m_ImageNeedsClearing = false;
			imageWasClearedLastPass = false;
		}
		else if (m_ImageNeedsClearing && !imageWasClearedLastPass)
		{
			imageWasClearedLastPass = true;
			return { true, true };
		}

		m_Camera->SingleThreadedRenderPass();

		return { true, false };
	}

	void DuelGraySpheres::OnImGuiRender()
	{
		ImGui::Begin("Camera Settings");
		if (ImGui::DragFloat3("Position", glm::value_ptr(m_CameraPosition), 0.1f))
		{
			m_ImageNeedsClearing = true;
			m_Camera->SetPosition(m_CameraPosition);
		}
		if (ImGui::DragFloat3("Rotation", glm::value_ptr(m_CameraRotation), 0.1f))
		{
			m_ImageNeedsClearing = true;
			m_Camera->SetRotate(m_CameraRotation);
		}
		if (ImGui::DragFloat("FOV", &m_FOV, 0.1f, 1.0f, 89.0f))
		{
			m_ImageNeedsClearing = true;
			m_Camera->SetFOV(m_FOV);
		}
		if (ImGui::DragFloat("Focal Length", &m_FocalLength, 0.1f, 0.1f, 100.0f))
		{
			m_ImageNeedsClearing = true;
			m_Camera->SetFocalLength(m_FocalLength);
		}
		ImGui::End();
	}

	bool DuelGraySpheres::OnEvent(BaseEvent& e)
	{
		EventDispatcher dispacher(e);

		dispacher.Dispatch<WindowResize>([this](const WindowResize&) {
			const auto& app = Application::GetConstInstance();
			this->m_Camera->SetScreenSize(Vec2(app.GetWindowWidth(), app.GetWindowHeight()));
			return false;
			});

		return false;
	}
}
