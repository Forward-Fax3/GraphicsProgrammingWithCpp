#pragma once
#include "Scene.hpp"


namespace OWC
{
    class RandTestScene : public BaseScene
    {
		enum class RenderMode : u8
        {
            RandomColorVec2 = 0,
			BlackAndWhiteNoiseVec2 = 1,
			RandColorVec4 = 2,
			BlackAndWhiteNoiseVec4 = 3
		};

    public:
		RandTestScene() = delete;
        explicit RandTestScene(std::vector<Vec4>& frameBuffer);
        ~RandTestScene() override = default;

        RandTestScene(const RandTestScene&) = delete;
        RandTestScene& operator=(const RandTestScene&) = delete;
        RandTestScene(RandTestScene&&) = delete;
        RandTestScene& operator=(RandTestScene&&) = delete;

        RenderPassReturnData RenderNextPass() override;

		void UpdateScreenSize(const Vec2u&) override;
		void OnImGuiRender() override;

    private:
		uSize GetStepSize() const;

    private:
		bool m_UpdateImage = true;
		uSize m_CurrentSampleIndex = 0;
		RenderMode m_RenderMode = RenderMode::RandomColorVec2;
    };
}
