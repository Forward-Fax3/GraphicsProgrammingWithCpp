#include "ImGuiLayer.hpp"
#include "Application.hpp"
#include "Renderer.hpp"

#include "BaseEvent.hpp"
#include "WindowResize.hpp"
#include "WindowMinimizeEvent.hpp"
#include "WindowRestoreEvent.hpp"

#include <imgui.h>
#include <array>
#include <chrono>


namespace OWC
{
	ImGuiLayer::ImGuiLayer()
	{
		ImGui::CreateContext();
		ImGui::StyleColorsDark();

		ImGuiIO& io = ImGui::GetIO();

		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		Application::GetInstance().GetWindow().ImGuiInit();
		m_RenderPassData = Graphics::Renderer::GetDynamicPass();
	}

	ImGuiLayer::~ImGuiLayer()
	{
		Application::GetInstance().GetWindow().ImGuiShutdown();
		ImGui::DestroyContext();
	}

	void ImGuiLayer::OnUpdate()
	{
		auto now = std::chrono::high_resolution_clock::now();
		m_DeltaTime = std::chrono::duration<float, std::milli>(now - m_LastTime).count();
		m_LastTime = now;
	}

	void ImGuiLayer::ImGuiRender()
	{
		ImGui::Begin("Frame rate");
		ImGui::Text("Delta Time: %.3f ms", m_DeltaTime);
		ImGui::Text("FPS: %2.1f", 1000.0f / m_DeltaTime);
		ImGui::End();
	}
	
	void ImGuiLayer::OnEvent(class BaseEvent& e)
	{
		EventDispatcher dispatcher(e);

		dispatcher.Dispatch<WindowRestore>([this](const WindowRestore&)
			{
				m_IsMinimized = false;
				return false;
			});

		if (m_IsMinimized)
			return;

		dispatcher.Dispatch<WindowMinimize>([this](const WindowMinimize&)
			{
				m_IsMinimized = true;
				return false;
			});

		dispatcher.Dispatch<WindowResize>([](const WindowResize& WRE)
			{
				ImGuiIO& io = ImGui::GetIO();
				io.DisplaySize = ImVec2(static_cast<float>(WRE.GetWidth()), static_cast<float>(WRE.GetHeight()));
				return false;
			});
	}
	
	void ImGuiLayer::Begin() const
	{
		Application::GetInstance().GetWindow().ImGuiNewFrame();
		ImGui::NewFrame();
	}
	
	void ImGuiLayer::End() const
	{
		using namespace Graphics;
		ImGui::Render();
		ImDrawData* drawData = ImGui::GetDrawData();

		std::array<std::string_view, 1> waitSemaphorenames = { "TestLayer" };
		std::array<std::string_view, 1> signalSemaphoreNames = { "ImGuiLayer" };

		static bool firstFrame = true;

		if (!firstFrame)
			Renderer::RestartRenderPass(m_RenderPassData);

		if (!m_IsMinimized)
		{
			Renderer::BeginDynamicPass(m_RenderPassData);
			Renderer::DrawImGui(m_RenderPassData, drawData);
			Renderer::EndPass(m_RenderPassData);
			Renderer::SubmitRenderPass(m_RenderPassData, waitSemaphorenames, signalSemaphoreNames);
		}

		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}

		firstFrame = false;
	}
}