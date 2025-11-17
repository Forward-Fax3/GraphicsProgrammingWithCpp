#pragma once
#include "GraphicsContext.hpp"


namespace OWC::Graphics
{
	class VulkanContext final : public GraphicsContext
	{
	public:
		explicit VulkanContext(const SDL_Window& windowHandle);
		~VulkanContext() override;

		void SwapBuffers() override;
		void WaitForIdle() override;
	};
}
