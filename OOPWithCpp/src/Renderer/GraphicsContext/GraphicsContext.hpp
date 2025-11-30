#pragma once
#include <memory>

#include <SDL3/SDL_video.h>


namespace OWC::Graphics
{
	class GraphicsContext
	{
	public:
		virtual ~GraphicsContext() = default;

		// delete copy/move constructor and copy/move assignment operator
		GraphicsContext(const GraphicsContext&) = delete;
		GraphicsContext& operator=(const GraphicsContext&) = delete;
		GraphicsContext(GraphicsContext&&) = delete;
		GraphicsContext& operator=(GraphicsContext&&) = delete;

		virtual void FinishRender() = 0;
		virtual void SwapPresentImage() = 0;

		virtual void WaitForIdle() = 0;
#ifndef DIST
		virtual void FlushValidationMessages() = 0;
#endif

		static std::unique_ptr<GraphicsContext> CreateGraphicsContext(SDL_Window& windowHandle);

	protected:
		GraphicsContext() = default; // protected constructor to prevent direct instantiation
	};
}
