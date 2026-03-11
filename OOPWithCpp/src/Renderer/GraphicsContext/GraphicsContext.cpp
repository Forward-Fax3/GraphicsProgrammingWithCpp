#include "GraphicsContext.hpp"
#include "VulkanContext.hpp"


namespace OWC::Graphics
{
	std::unique_ptr<GraphicsContext> GraphicsContext::CreateGraphicsContext(SDL_Window& windowHandle, const WindowProperties& properties)
	{
		return std::make_unique<VulkanContext>(windowHandle, properties);
	}
}
