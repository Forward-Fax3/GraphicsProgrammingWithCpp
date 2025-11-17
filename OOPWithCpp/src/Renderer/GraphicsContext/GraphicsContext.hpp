#pragma once
#include <memory>
#include <SDL3/SDL.h>

#include "core.hpp"


namespace OWC::Graphics
{
	class GraphicsContext
	{
	public:
		virtual ~GraphicsContext() = default;

		virtual void SwapBuffers() = 0;

		virtual void WaitForIdle() = 0;

		static std::unique_ptr<GraphicsContext> CreateGraphicsContext(const SDL_Window& windowHandle);

	protected:
		GraphicsContext() = default; // protected constructor to prevent direct instantiation
	};
}
