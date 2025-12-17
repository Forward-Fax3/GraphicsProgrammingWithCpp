#pragma once
#include "Layer.hpp"
#include "InterLayerData.hpp"
#include "ImageLoader.hpp"
#include "Scene.hpp"

#include <memory>
#include <bitset>


namespace OWC
{
	enum class GammaCorrection : uint8_t
	{
		None = 0,
		Gamma19,
		Gamma20,
		Gamma22,
		Gamma24,
		GammaBT1886,
		custom
	};

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
		void UpdateGammaValue(GammaCorrection gammaCorrection) const;

	private:
		std::shared_ptr<InterLayerData> m_InterLayerData = nullptr;
		bool m_RayTracingStateUpdated = false;
		bool m_ToggleRaytracedImage = false;

		std::unique_ptr<BaseScene> m_Scene;
	};
}
