#pragma once


namespace OWC
{
	class Layer
	{
	public:
		Layer() = default;
		virtual ~Layer() = default;
		Layer(const Layer&) = delete;
		Layer& operator=(const Layer&) = delete;
		Layer(Layer&&) = delete;
		Layer&& operator=(Layer&&) = delete;

		virtual void OnUpdate() = 0;
		virtual void ImGuiRender() = 0;
		virtual void OnEvent(class BaseEvent& event) = 0;

		[[nodiscard]] bool IsActive() const { return m_Active; }
		void SetActive(bool active) { m_Active = active; }

	private:
		bool m_Active = true;
	};
}
