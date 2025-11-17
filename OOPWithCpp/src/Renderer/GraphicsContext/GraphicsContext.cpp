#include "GraphicsContext.hpp"
#include "VulkanContext.hpp"


namespace OWC::Graphics
{
	std::unique_ptr<GraphicsContext> GraphicsContext::CreateGraphicsContext(const SDL_Window& windowHandle)
	{
		return std::make_unique<VulkanContext>(windowHandle);
	}
}
