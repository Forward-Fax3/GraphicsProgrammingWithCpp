#include "Application.hpp"
#include "BasicScene.hpp"
#include "Sphere.hpp"
#include "Ray.hpp"
#include "OWCRand.hpp"

#include "BaseEvent.hpp"
#include "WindowResize.hpp"

#include "Lambertian.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/vec_swizzle.hpp>
#include <glm/gtc/type_ptr.hpp>


namespace OWC
{
	BasicScene::BasicScene(std::vector<Vec4>& frameBuffer)
		: BaseScene(frameBuffer)
	{
		m_Hittables = std::make_shared<Sphere>(Vec3(0.0f), 1.0f,
			std::make_shared<Lambertian>(Colour(1.0f, 0.0f, 0.0f, 1.0))
		);

		m_Camera = std::make_unique<RTCamera>(m_Hittables, frameBuffer);
		m_Camera->SetPosition(m_CameraPosition);
		m_Camera->SetRotate(m_CameraRotation);
		m_Camera->SetFOV(m_FOV);
		m_Camera->SetFocalLength(m_FocalLength);
	}

	RenderPassReturnData BasicScene::RenderNextPass()
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

	void BasicScene::OnImGuiRender()
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

	bool BasicScene::OnEvent(BaseEvent& e)
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