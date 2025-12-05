#pragma once
#include "Layer.hpp"
#include "Renderer.hpp"

#include <chrono>


namespace OWC
{
	class ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer() override;
		ImGuiLayer(ImGuiLayer&) = delete;
		ImGuiLayer& operator=(ImGuiLayer&) = delete;
		ImGuiLayer(ImGuiLayer&&) = delete;
		ImGuiLayer&& operator=(ImGuiLayer&&) = delete;

		void OnUpdate() override;
		void ImGuiRender() override;
		void OnEvent(class BaseEvent& event) override;

		void Begin() const;
		void End() const;

	private:
		std::shared_ptr<Graphics::RenderPassData> m_RenderPassData = nullptr;
		std::chrono::steady_clock::time_point m_LastTime;
		float m_DeltaTime = 0.0f;
		bool m_IsMinimized = false;
	};
}
