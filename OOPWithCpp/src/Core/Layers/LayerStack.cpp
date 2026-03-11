#include "LayerStack.hpp"

#include <ranges>


namespace OWC
{
	void LayerStack::OnUpdate() const
	{
		for (const auto& layer : m_Layers)
			if (layer->IsActive())
				layer->OnUpdate();
	}

	void LayerStack::ImGuiRender() const
	{
		for (const auto& layer : m_Layers)
			if (layer->IsActive())
				layer->ImGuiRender();
	}

	void LayerStack::OnEvent(BaseEvent& event) const
	{
		// top-down
		for (const auto& layer : std::ranges::reverse_view(m_Layers))
		{
			layer->OnEvent(event);
			if (event.HasBeenHandled())
				break;
		}
	}
}