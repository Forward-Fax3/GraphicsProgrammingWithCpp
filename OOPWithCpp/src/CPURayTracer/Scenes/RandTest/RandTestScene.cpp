#include "RandTestScene.hpp"
#include "OWCRand.hpp"

#include "BaseEvent.hpp"
#include "WindowResize.hpp"

#include <array>
#include <utility>

#include <imgui.h>

#include <glm/gtx/vec_swizzle.hpp>


namespace OWC
{
	RandTestScene::RandTestScene(std::vector<Vec4>& frameBuffer)
		: BaseScene(frameBuffer) {}

	RenderPassReturnData RandTestScene::RenderNextPass()
	{
		if (!m_UpdateImage)
			return { false, false };

		m_CurrentSampleIndex++;
		auto& frameBuffer = GetFrameBuffer();
		uSize step = GetStepSize();
		auto numberOfSamples = static_cast<f32>(m_CurrentSampleIndex);

		uSize i = 0;
		for (; i < frameBuffer.size(); i += step)
		{
			switch (m_RenderMode)
			{
			case OWC::RandTestScene::RenderMode::RandomColorVec2:
			{
				frameBuffer[i] = Vec4(Rand::LinearFastRandVec2(Vec2(0.0f), Vec2(1.0f)), 0.0f, 1.0f) * numberOfSamples;
				break;
			}
			case OWC::RandTestScene::RenderMode::BlackAndWhiteNoiseVec2:
			{
				Vec2 noise = Rand::LinearFastRandVec2(Vec2(0.0f), Vec2(1.0f));
				frameBuffer[i + 0] = Vec4(glm::xxx(noise), 1.0f) * numberOfSamples;
				frameBuffer[i + 1] = Vec4(glm::yyy(noise), 1.0f) * numberOfSamples;
				break;
			}
			case OWC::RandTestScene::RenderMode::RandColorVec4:
			{
				frameBuffer[i] = Vec4(Rand::LinearFastRandVec4(Vec4(0.0f), Vec4(1.0f)));
				frameBuffer[i].a = 1.0f;
				frameBuffer[i] *= numberOfSamples;
				break;
			}
			case OWC::RandTestScene::RenderMode::BlackAndWhiteNoiseVec4:
			{
				Vec4 noise = Rand::LinearFastRandVec4(Vec4(0.0f), Vec4(1.0f));
				frameBuffer[i + 0] = Vec4(glm::xxx(noise), 1.0f) * numberOfSamples;
				frameBuffer[i + 1] = Vec4(glm::yyy(noise), 1.0f) * numberOfSamples;
				frameBuffer[i + 2] = Vec4(glm::zzz(noise), 1.0f) * numberOfSamples;
				frameBuffer[i + 3] = Vec4(glm::www(noise), 1.0f) * numberOfSamples;
				break;
			}
			default:
				break;
			}
		}

		if (step > 1 && i != frameBuffer.size())
		{
			i -= step;
			switch (m_RenderMode)
			{
			case OWC::RandTestScene::RenderMode::BlackAndWhiteNoiseVec2:
			{
				Vec2 noise = Rand::LinearFastRandVec2(Vec2(0.0f), Vec2(1.0f)) * numberOfSamples;
				frameBuffer[i] = Vec4(glm::xxx(noise), 1.0f);
				break;
			}
			case OWC::RandTestScene::RenderMode::BlackAndWhiteNoiseVec4:
			{
				Vec4 noise = Rand::LinearFastRandVec4(Vec4(0.0f), Vec4(1.0f)) * numberOfSamples;

				uSize remaining = frameBuffer.size() - i;

				for (uSize j = 0; j < remaining; ++j)
				{
					Vec3 jVec3(noise[j]);
					frameBuffer[i + j] = Vec4(jVec3, 1.0f);
				}

				break;
			}
			case OWC::RandTestScene::RenderMode::RandomColorVec2:
			case OWC::RandTestScene::RenderMode::RandColorVec4:
			default:
				std::unreachable();
			}
		}

		return { true, false };
	}

	void RandTestScene::OnImGuiRender()
	{
		ImGui::Begin("Rand Test Scene");
		m_UpdateImage = ImGui::Button("Randomize Framebuffer");
		constexpr std::array<const char*, 4> renderModeNames = {
			"Random Color Vec2",
			"Black and White Noise Vec2",
			"Random Color Vec4",
			"Black and White Noise Vec4"
		};
		static i32 currentRenderModeIndex = 0;
		if (ImGui::Combo("Render Mode", &currentRenderModeIndex, renderModeNames.data(), static_cast<i32>(renderModeNames.size())))
			m_RenderMode = static_cast<RenderMode>(currentRenderModeIndex);
		ImGui::End();
	}

	bool RandTestScene::OnEvent(BaseEvent& event)
	{
		EventDispatcher dispatcher(event);

		dispatcher.Dispatch<WindowResize>([this](const WindowResize&) {
			this->m_CurrentSampleIndex = 0;
			return false;
			});

		return false;
	}

	uSize RandTestScene::GetStepSize() const
	{
		switch (m_RenderMode)
		{
		case RenderMode::RandomColorVec2:
			return 1;
		case RenderMode::BlackAndWhiteNoiseVec2:
			return 2;
		case RenderMode::RandColorVec4:
			return 1;
		case RenderMode::BlackAndWhiteNoiseVec4:
			return 4;
		default:
			return 1;
		}
	}
}
