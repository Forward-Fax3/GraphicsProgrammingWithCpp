#pragma once
#include <memory>
#include <bitset>

#include "Window.hpp"
#include "BaseEvent.hpp"
#include "LayerStack.hpp"
#include "BaseShader.hpp"

#include "ImGuiLayer.hpp"


namespace OWC
{
	class Application
	{
	public:
		Application() = delete;
		explicit Application(std::bitset<2>& runFlags);
		~Application();

		// delete copy/move constructor and copy/move assignment operator
		Application(const Application&) = delete;
		Application& operator=(const Application&) = delete;
		Application(Application&&) = delete;
		Application& operator=(Application&&) = delete;

		void Run() const;

		OWC_FORCE_INLINE void PushLayer(const std::shared_ptr<Layer>& layer) { m_LayerStack->PushLayer(layer); }
		OWC_FORCE_INLINE void PushOverlay(const std::shared_ptr<Layer>& overlay) { m_LayerStack->PushOverlay(overlay); }
		OWC_FORCE_INLINE void PopLayer(const std::shared_ptr<Layer>& layer) { m_LayerStack->PopLayer(layer); }
		OWC_FORCE_INLINE void PopOverlay(const std::shared_ptr<Layer>& overlay) { m_LayerStack->PopLayer(overlay); }

		OWC_FORCE_INLINE void Stop() { m_RunFlags.set(0, false); } // set run flag to shutdown
		OWC_FORCE_INLINE void Restart() { Stop(); m_RunFlags.set(1, true); } // Set shutdown and restart flags

		static Application& GetInstance() { return *s_Instance; }
		static const Application& GetConstInstance() { return *s_Instance; }

		OWC_FORCE_INLINE Window& GetWindow() const { return *m_Window; }

		OWC_FORCE_INLINE u32 GetWindowWidth() const { return m_Window->GetWidth(); }
		OWC_FORCE_INLINE u32 GetWindowHeight() const { return m_Window->GetHeight(); }
		OWC_FORCE_INLINE Vec2u GetWindowSize() const { return { m_Window->GetWidth(), m_Window->GetHeight() }; }

	private:
		void OnEvent(BaseEvent& event) const;

	private:
		std::unique_ptr<Window> m_Window = nullptr;
		std::unique_ptr<LayerStack> m_LayerStack = nullptr;
		std::bitset<2>& m_RunFlags; // Bit 0: Application running, Bit 1: restart application
		std::unique_ptr<Graphics::BaseShader> m_Shader = nullptr;
		std::shared_ptr<ImGuiLayer> m_ImGuiLayer = nullptr;

		static Application* s_Instance;
	};
}
