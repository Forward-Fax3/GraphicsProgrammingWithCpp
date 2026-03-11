#pragma once
#include "Core.hpp"
#include <vector>
#include <memory>

#include "Layer.hpp"
#include "BaseEvent.hpp"


namespace OWC
{
	class LayerStack
	{
	public:
		OWC_FORCE_INLINE LayerStack()
		{
			// preallocate some memory to avoid multiple allocations
			m_Layers.reserve(16);
		}
		OWC_FORCE_INLINE ~LayerStack()
		{
			m_Layers.clear();
		}

		// delete copy constructor and copy assignment operator
		LayerStack(LayerStack&) = delete;
		LayerStack& operator=(LayerStack&) = delete;

		// delete move constructor and move assignment operator
		LayerStack(LayerStack&&) = delete;
		LayerStack& operator=(LayerStack&&) = delete;

		OWC_FORCE_INLINE void PushLayer(const std::shared_ptr<Layer>& layer)
		{
			m_Layers.emplace(m_Layers.begin() + m_LayerInsertIndex, layer);
			m_LayerInsertIndex++;
		}
		
		OWC_FORCE_INLINE void PopLayer(const std::shared_ptr<Layer>& layer)
		{
			m_Layers.erase(std::ranges::find(m_Layers, layer));
		}

		OWC_FORCE_INLINE void PushOverlay(const std::shared_ptr<Layer>& overlay)
		{
			m_Layers.emplace_back(overlay);
		}

		OWC_FORCE_INLINE void ClearLayers()
		{
			m_Layers.clear();
		}

		void OnUpdate() const;
		void ImGuiRender() const;
		void OnEvent(BaseEvent& event) const;

	private:
		std::vector<std::shared_ptr<Layer>> m_Layers;
		i32 m_LayerInsertIndex = 0;
	};
}
