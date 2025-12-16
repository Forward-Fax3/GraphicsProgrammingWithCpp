#pragma once
#include "Layer.hpp"
#include "InterLayerData.hpp"
#include "ImageLoader.hpp"
#include "Scene.hpp"

#include <memory>
#include <bitset>


namespace OWC
{
	class CPURayTracer : public Layer
	{
	public:
		CPURayTracer() = delete;
		explicit CPURayTracer(const std::shared_ptr<InterLayerData>& interLayerData);
		~CPURayTracer() override = default;

		CPURayTracer(const CPURayTracer&) = delete;
		CPURayTracer& operator=(const CPURayTracer&) = delete;
		CPURayTracer(CPURayTracer&&) = delete;
		CPURayTracer&& operator=(CPURayTracer&&) = delete;

		void OnUpdate() override;
		void ImGuiRender() override;
		void OnEvent(class BaseEvent& event) override;

	private:
		std::shared_ptr<InterLayerData> m_InterLayerData = nullptr;
		std::unique_ptr<ImageLoader> m_ImageLoader = nullptr;
		std::unique_ptr<ImageLoader> m_SecondImageLoader = nullptr;
		bool m_ToggleImage = false;
		bool m_ImageStateUpdated = false;
		bool m_ToggleSecondImage = false;
		bool m_SecondImageStateUpdated = false;
		bool m_ToggleRaytracedImage = false;
		bool m_ShowRaytracedImage = false;

		std::unique_ptr<BaseScene> m_Scene;
	};
}
