#pragma once
#include "Core.hpp"
#include "Scene.hpp"

#include "Hittables.hpp"


namespace OWC
{
	class Book1FinalRender : public BaseScene
	{
	public:
		Book1FinalRender();
		~Book1FinalRender() override = default;

		Book1FinalRender(const Book1FinalRender&) = delete;
		Book1FinalRender& operator=(const Book1FinalRender&) = delete;
		Book1FinalRender(Book1FinalRender&&) = delete;
		Book1FinalRender& operator=(Book1FinalRender&&) = delete;

		void SetBaseCameraSettings(CameraRenderSettings& cameraSettings) const override;

		const std::shared_ptr<BaseHitable>& GetHitable() override { return m_Hitable; }

	private:
		std::shared_ptr<BaseHitable> m_Hitable;
		std::shared_ptr<Hitables> m_SceneObjects;
	};
}
