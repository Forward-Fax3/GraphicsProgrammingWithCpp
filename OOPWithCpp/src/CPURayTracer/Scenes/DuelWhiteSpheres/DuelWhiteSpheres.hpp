#pragma once
#include "Scene.hpp"
#include "Camera.hpp"

#include "Hittables.hpp"


namespace OWC
{
	class DuelGraySpheres : public BaseScene
	{
	public:
		DuelGraySpheres() = delete;
		explicit DuelGraySpheres(std::vector<Vec4>& frameBuffer);
		~DuelGraySpheres() override = default;

		DuelGraySpheres(const DuelGraySpheres&) = delete;
		DuelGraySpheres& operator=(const DuelGraySpheres&) = delete;
		DuelGraySpheres(DuelGraySpheres&&) = delete;
		DuelGraySpheres& operator=(DuelGraySpheres&&) = delete;

		RenderPassReturnData RenderNextPass() override;

		void UpdateScreenSize(const Vec2u& newSize) override;
		void OnImGuiRender() override;

	private:
		std::shared_ptr<Hitables> m_SceneObjects = nullptr;
		std::unique_ptr<RTCamera> m_Camera = nullptr;
		Vec3 m_CameraRotation{ 0.0 };
		Vec3 m_CameraPosition{ 0.0, 0.0, 5.0 };
		f32 m_FOV = 45.0f;
		f32 m_FocalLength = 45.0f;
		bool m_ImageNeedsClearing = false;
	};
}
