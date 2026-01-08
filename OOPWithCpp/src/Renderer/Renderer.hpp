#pragma once
#define RENDERER_HPP
#ifndef GRAPHICSCONTEXT_HPP
#include "GraphicsContext.hpp"
#endif
#include "BaseShader.hpp"
#include "UniformBuffer.hpp"

#include <imgui.h>
#include <span>


namespace OWC::Graphics
{
	class GraphicsContext;

	enum class RendererAPI
	{
//		None = 0,
		Vulkan = 1,
	}; // only Vulkan is supported for now

	enum class RenderPassType
	{
		Static = 0,
		Dynamic = 1,
	};

	class Renderer;

	class RenderPassData
	{
		friend class Renderer;

	public:
		virtual ~RenderPassData() = default;

	private:
		void virtual BeginDynamicPass() = 0;
		void virtual AddPipeline(const BaseShader& shader) = 0;
		void virtual BindUniform(const BaseShader& shader) = 0;
		void virtual BindTexture(const BaseShader& shader, u32 binding, u32 textureID) = 0;
		void virtual BindDynamicTexture(const BaseShader& shader, u32 binding, u32 textureID) = 0;
		void virtual Draw(u32 vertexCount, u32 instanceCount = 1, u32 firstVertex = 0, u32 firstInstance = 0) = 0;
		void virtual EndRenderPass() = 0;
		void virtual submitRenderPass(std::span<std::string_view> waitSemaphoreNames, std::span<std::string_view> startSemaphore) = 0;

		void virtual RestartRenderPass() = 0;

		void virtual DrawImGui(ImDrawData* drawData) = 0;

		uSize virtual GetNumberOfFramesInFlight() const = 0;

	protected:
		explicit RenderPassData(RenderPassType type) : type(type) {}
		// delete copy/move constructor and copy/move assignment operator
		RenderPassData(const RenderPassData&) = delete;
		RenderPassData& operator=(const RenderPassData&) = delete;
		RenderPassData(RenderPassData&&) = delete;
		RenderPassData& operator=(RenderPassData&&) = delete;

		RenderPassType GetRenderPassType() const { return type; }

	private:
		RenderPassType type;
	};

	class Renderer
	{
	public:
		static void Init();
		static void Shutdown();
		static void FinishRender();

		static std::shared_ptr<RenderPassData> GetDynamicPass();
		static void BeginDynamicPass(const std::shared_ptr<RenderPassData>& data);

		static std::shared_ptr<RenderPassData> BeginPass();
		static void PipelineBind(const std::shared_ptr<RenderPassData>& data, const BaseShader& shader);
		static void BindUniform(const std::shared_ptr<RenderPassData>& data, const BaseShader& shader);
		static void BindTexture(const std::shared_ptr<RenderPassData>& data, const BaseShader& shader, u32 binding, u32 textureID);
		static void BindDynamicTexture(const std::shared_ptr<RenderPassData>& data, const BaseShader& shader, u32 binding, u32 textureID);
		static void Draw(const std::shared_ptr<RenderPassData>& data, u32 vertexCount, u32 instanceCount = 1, u32 firstVertex = 0, u32 firstInstance = 0);
		static void EndPass(const std::shared_ptr<RenderPassData>& data);
		static void SubmitRenderPass(const std::shared_ptr<RenderPassData>& data, std::span<std::string_view> waitSemaphoreNames, std::span<std::string_view> startSemaphoreNames);

		static void RestartRenderPass(const std::shared_ptr<RenderPassData>& data);

		static void DrawImGui(const std::shared_ptr<RenderPassData>& data, ImDrawData* drawData);

		static uSize GetNumberOfFramesInFlight(const std::shared_ptr<RenderPassData>& data);

		static inline RendererAPI GetAPI() { return s_API; }

	private:
		static std::shared_ptr<GraphicsContext> s_GC;
		static RendererAPI s_API;
	};
}
