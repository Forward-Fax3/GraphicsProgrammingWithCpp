#pragma once
#include "Renderer.hpp"
#include "BaseShader.hpp"

#include "vulkan/vulkan.hpp"


namespace OWC::Graphics
{
	class VulkanRenderPass : public RenderPassData
	{
	public:
		VulkanRenderPass() = delete;
		explicit VulkanRenderPass(RenderPassType type);
		~VulkanRenderPass() override;

		VulkanRenderPass(const VulkanRenderPass&) = delete;
		VulkanRenderPass& operator=(const VulkanRenderPass&) = delete;
		VulkanRenderPass(VulkanRenderPass&&) = delete;
		VulkanRenderPass& operator=(VulkanRenderPass&&) = delete;

		vk::Fence GetFence() const { return m_Fence; }

	private:
		void BeginDynamicPass() override;
		void AddPipeline(const BaseShader& shader) override;
		void BindUniform(const BaseShader& shader) override;
		void BindTexture(const BaseShader& shader, u32 binding, u32 textureID) override;
		void BindDynamicTexture(const BaseShader& shader, u32 binding, u32 textureID) override;
		void Draw(u32 vertexCount, u32 instanceCount = 1, u32 firstVertex = 0, u32 firstInstance = 0) override;
		void EndRenderPass() override;
		void submitRenderPass(std::span<std::string_view> waitSemaphoreNames, std::span<std::string_view> startSemaphore) override;

		void RestartRenderPass() override;

		void DrawImGui(ImDrawData* drawData) override;

		uSize GetNumberOfFramesInFlight() const override;

	private:
		std::vector<vk::CommandBuffer> m_CommandBuffers = {};
		vk::Fence m_Fence = vk::Fence();
	};
}
