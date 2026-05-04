#include "VulkanRenderPass.hpp"
#include "Application.hpp"

#include "VulkanCore.hpp"
#include "VulkanShader.hpp"
#include "VulkanUniformBuffer.hpp"

#include <backends/imgui_impl_vulkan.h>


namespace OWC::Graphics
{
	VulkanRenderPass::VulkanRenderPass(const RenderPassType type)
		: RenderPassData(type)
	{
		const VulkanCore& vkCore = VulkanCore::GetConstInstance();

		const vk::Rect2D rect(
			vk::Offset2D(0, 0),
			{
				Application::GetConstInstance().GetPixelWidth(),
				Application::GetConstInstance().GetPixelHeight()
			});

		if (type == RenderPassType::Static)
		{
			m_CommandBuffers = vkCore.GetGraphicsCommandBuffer();
			for (uSize i = 0; i < m_CommandBuffers.size(); ++i)
			{
				vk::CommandBuffer cmdBuf = m_CommandBuffers[i];
				cmdBuf.begin(vk::CommandBufferBeginInfo());

				const std::array renderingAttachmentInfo = { vk::RenderingAttachmentInfo()
					.setImageView(vkCore.GetSwapchainImageViews()[i])
					.setImageLayout(vk::ImageLayout::eColorAttachmentOptimal)
					.setLoadOp(vk::AttachmentLoadOp::eLoad)
					.setStoreOp(vk::AttachmentStoreOp::eStore)
				};

				cmdBuf.beginRendering(vk::RenderingInfo()
					.setRenderArea(rect)
					.setLayerCount(1)
					.setColorAttachmentCount(1)
					.setColorAttachments(renderingAttachmentInfo)
				);

				cmdBuf.setViewport(0, vk::Viewport(0.0f, 0.0f,
					static_cast<f32>(Application::GetConstInstance().GetPixelWidth()),
					static_cast<f32>(Application::GetConstInstance().GetPixelHeight()),
					0.0f, 1.0f));
				cmdBuf.setScissor(0, rect);
			}
		}
		else if (type == RenderPassType::Dynamic)
			m_CommandBuffers = vkCore.GetDynamicGraphicsCommandBuffer();
		else if (type == RenderPassType::StaticRayTracing)
		{
			m_CommandBuffers = vkCore.GetComputeCommandBuffer();
			for (uSize i = 0; i < m_CommandBuffers.size(); ++i)
			{
				vk::CommandBuffer cmdBuf = m_CommandBuffers[i];
				cmdBuf.begin(vk::CommandBufferBeginInfo());
			}
		}
		else if (type == RenderPassType::DynamicRayTracing)
			m_CommandBuffers = vkCore.GetDynamicComputeCommandBuffer();
		else
			Log<LogLevel::Critical>("Invalid RenderPassType specified when creating VulkanRenderPass");
	}

	VulkanRenderPass::~VulkanRenderPass()
	{
		const VulkanCore& vkCore = VulkanCore::GetConstInstance();
		const vk::Device& device = vkCore.GetDevice();

		if (GetRenderPassType() == RenderPassType::Static)
			for (auto& cmdBuf : m_CommandBuffers)
				device.freeCommandBuffers(vkCore.GetGraphicsCommandPool(), cmdBuf);
		else if (GetRenderPassType() == RenderPassType::Dynamic)
			for (auto& cmdBuf : m_CommandBuffers)
				device.freeCommandBuffers(vkCore.GetDynamicGraphicsCommandPool(), cmdBuf);
		else if (GetRenderPassType() == RenderPassType::StaticRayTracing)
			for (auto& cmdBuf : m_CommandBuffers)
				device.freeCommandBuffers(vkCore.GetComputeCommandPool(), cmdBuf);
		else if (GetRenderPassType() == RenderPassType::DynamicRayTracing)
			for (auto& cmdBuf : m_CommandBuffers)
				device.freeCommandBuffers(vkCore.GetDynamicComputeCommandPool(), cmdBuf);
		else
			std::unreachable();
	}

	void VulkanRenderPass::PushConstant(const BaseShader& shader, uSize size, const void* dataPtr)
	{
		const auto info = vk::PushConstantsInfo()
			.setLayout(dynamic_cast<const VulkanBaseShader&>(shader).GetPipelineLayout())
			.setStageFlags(vk::ShaderStageFlagBits::eClosestHitKHR | vk::ShaderStageFlagBits::eRaygenKHR | vk::ShaderStageFlagBits::eMissKHR)
			.setOffset(0)
			.setSize(size)
			.setPValues(dataPtr);

		const auto& vulkanShader = dynamic_cast<const VulkanBaseShader&>(shader);
		if (testBitMask(GetRenderPassType(), RenderPassType::Dynamic))
			m_CommandBuffers[VulkanCore::GetConstInstance().GetCurrentFrameIndex()].pushConstants2(info);
		else
			for (const auto& cmdBuf : m_CommandBuffers)
				cmdBuf.pushConstants2(info);
	}

	void VulkanRenderPass::BeginDynamicPass()
	{
		if (testBitMask(GetRenderPassType(), RenderPassType::Static))
		{
			Log<LogLevel::Error>("BeginDynamicPass is not needed for static render, this is due to begin code already being done in the constructor");
			return;
		}

		VulkanCore& vkCore = VulkanCore::GetInstance();
		const uSize frameIndex = vkCore.GetCurrentFrameIndex();

		const vk::CommandBuffer& cmdBuf = m_CommandBuffers[frameIndex];
		cmdBuf.reset(vk::CommandBufferResetFlags());

		cmdBuf.begin(vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));

		if ((GetRenderPassType() & RenderPassType::RayTracingBit) == RenderPassType::RayTracingBit)
			return;

		const std::array renderingAttachmentInfo = {
			vk::RenderingAttachmentInfo()
				.setImageView(vkCore.GetSwapchainImageViews()[frameIndex])
				.setImageLayout(vk::ImageLayout::eColorAttachmentOptimal)
				.setLoadOp(vk::AttachmentLoadOp::eLoad)
				.setStoreOp(vk::AttachmentStoreOp::eStore)
		};

		const vk::Rect2D rect(
			vk::Offset2D(0, 0),
			{
				Application::GetConstInstance().GetPixelWidth(),
				Application::GetConstInstance().GetPixelHeight()
			});

		cmdBuf.beginRendering(vk::RenderingInfo()
			.setRenderArea(rect)
			.setLayerCount(1)
			.setColorAttachmentCount(1)
			.setColorAttachments(renderingAttachmentInfo)
		);

		cmdBuf.setViewport(0, vk::Viewport(0.0f, 0.0f,
			static_cast<f32>(Application::GetConstInstance().GetPixelWidth()),
			static_cast<f32>(Application::GetConstInstance().GetPixelHeight()),
			0.0f, 1.0f));
		cmdBuf.setScissor(0, rect);
	}

	void VulkanRenderPass::AddPipeline(const BaseShader& shader)
	{
		const auto bindPoint = testBitMask(GetRenderPassType(), RenderPassType::RayTracingBit) ? vk::PipelineBindPoint::eRayTracingKHR : vk::PipelineBindPoint::eGraphics;

		const auto& vulkanShader = dynamic_cast<const VulkanBaseShader&>(shader);
		if (testBitMask(GetRenderPassType(), RenderPassType::Dynamic))
		{
			const uSize frameIndex = VulkanCore::GetConstInstance().GetCurrentFrameIndex();
			m_CommandBuffers[frameIndex].bindPipeline(bindPoint, vulkanShader.GetPipeline());
		}
		else
			for (auto& cmdBuf : m_CommandBuffers)
				cmdBuf.bindPipeline(bindPoint, vulkanShader.GetPipeline());
	}

	void VulkanRenderPass::BindUniform(const BaseShader& shader)
	{
		const auto bindPoint = testBitMask(GetRenderPassType(), RenderPassType::RayTracingBit) ? vk::PipelineBindPoint::eRayTracingKHR : vk::PipelineBindPoint::eGraphics;

		const auto& vulkanShader = dynamic_cast<const VulkanBaseShader&>(shader);
		if (testBitMask(GetRenderPassType(), RenderPassType::Dynamic))
			m_CommandBuffers[VulkanCore::GetConstInstance().GetCurrentFrameIndex()].bindDescriptorSets(
				bindPoint,
				vulkanShader.GetPipelineLayout(),
				0,
				vulkanShader.GetDescriptorSet(),
				{}
			);
		else
			for (const auto& cmdBuf : m_CommandBuffers)
				cmdBuf.bindDescriptorSets(
					bindPoint,
					vulkanShader.GetPipelineLayout(),
					0,
					vulkanShader.GetDescriptorSet(),
					{}
				);
	}

	void VulkanRenderPass::BindTexture(const BaseShader& shader, u32 binding, u32 textureID)
	{
		const auto bindPoint = testBitMask(GetRenderPassType(), RenderPassType::RayTracingBit) ? vk::PipelineBindPoint::eRayTracingKHR : vk::PipelineBindPoint::eGraphics;

		(void)binding;
		(void)textureID;

		const auto& vulkanShader = dynamic_cast<const VulkanBaseShader&>(shader);

		if (testBitMask(GetRenderPassType(), RenderPassType::Dynamic))
			m_CommandBuffers[VulkanCore::GetConstInstance().GetCurrentFrameIndex()].bindDescriptorSets(
				bindPoint,
				vulkanShader.GetPipelineLayout(),
				0,
				vulkanShader.GetDescriptorSet(),
				{}
			);
		else
			for (const auto& cmdBuf : m_CommandBuffers)
				cmdBuf.bindDescriptorSets(
					bindPoint,
					vulkanShader.GetPipelineLayout(),
					0,
					vulkanShader.GetDescriptorSet(),
					{}
				);
	}

	void VulkanRenderPass::Draw(u32 vertexCount, u32 instanceCount /*= 1*/, u32 firstVertex /*= 0*/, u32 firstInstance /*= 0*/)
	{
		if (testBitMask(GetRenderPassType(), RenderPassType::RayTracingBit))
		{
			Log<LogLevel::Error>("Draw called on a ray tracing render pass. This is not allowed.");
			return;
		}

		if (GetRenderPassType() == RenderPassType::Dynamic)
		{
			const uSize frameIndex = VulkanCore::GetConstInstance().GetCurrentFrameIndex();
			m_CommandBuffers[frameIndex].draw(vertexCount, instanceCount, firstVertex, firstInstance);
		}
		else
			for (auto& cmdBuf : m_CommandBuffers)
				cmdBuf.draw(vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void VulkanRenderPass::RayTrace(const BaseShader& shader, u32 depth)
	{
		if (!testBitMask(GetRenderPassType(), RenderPassType::RayTracingBit))
		{
			Log<LogLevel::Error>("RayTrace called on a non ray tracing render pass. This is not allowed.");
			return;
		}

		const auto& vulkanShader = dynamic_cast<const VulkanRayTracingShader&>(shader);

		if (testBitMask(GetRenderPassType(), RenderPassType::Dynamic))
			m_CommandBuffers[VulkanCore::GetConstInstance().GetCurrentFrameIndex()].traceRaysKHR(
				&vulkanShader.GetRayGenShaderSBTEntry(),
				&vulkanShader.GetMissGroupSBTEntry(),
				&vulkanShader.GetHitGroupSBTEntry(),
				&vulkanShader.GetCallableGroupSBTEntry(),
				Application::GetConstInstance().GetPixelWidth(),
				Application::GetConstInstance().GetPixelHeight(),
				depth
			);
		else
			for (auto& cmdBuf : m_CommandBuffers)
				cmdBuf.traceRaysKHR(
					&vulkanShader.GetRayGenShaderSBTEntry(),
					&vulkanShader.GetMissGroupSBTEntry(),
					&vulkanShader.GetHitGroupSBTEntry(),
					&vulkanShader.GetCallableGroupSBTEntry(),
					Application::GetConstInstance().GetPixelWidth(),
					Application::GetConstInstance().GetPixelHeight(),
					depth
				);
	}

	void VulkanRenderPass::EndRenderPass()
	{
		if (GetRenderPassType() == RenderPassType::Static)
			for (uSize i = 0; i != VulkanCore::GetConstInstance().GetSwapchainImages().size(); i++)
			{
				vk::CommandBuffer& cmdBuf = m_CommandBuffers[i];
				cmdBuf.endRendering();
				cmdBuf.end();
			}
		else
		{
			const VulkanCore& vkCore = VulkanCore::GetInstance();
			const uSize i = vkCore.GetCurrentFrameIndex();
			const vk::CommandBuffer& cmdBuf = m_CommandBuffers[i];
			if (!testBitMask(GetRenderPassType(), RenderPassType::RayTracingBit))
				cmdBuf.endRendering();
			cmdBuf.end();
		}
	}

	void VulkanRenderPass::submitRenderPass(std::span<std::string_view> waitSemaphoreNames, std::span<std::string_view> startSemaphore)
	{
		VulkanCore& vkCore = VulkanCore::GetInstance();

		std::vector<vk::Semaphore> waitSemaphores = vkCore.GetSemaphoresFromNames(waitSemaphoreNames);
		std::vector<vk::Semaphore> signalSemaphores = vkCore.GetSemaphoresFromNames(startSemaphore);

		vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
		vk::SubmitInfo submitInfo = vk::SubmitInfo()
			.setWaitDstStageMask(waitDestinationStageMask)
			.setCommandBuffers(m_CommandBuffers[vkCore.GetCurrentFrameIndex()])
			.setCommandBufferCount(1);

		if (!waitSemaphores.empty())
			submitInfo.setWaitSemaphores(waitSemaphores);
		else
			submitInfo.setWaitSemaphores(VK_NULL_HANDLE);

		if (!signalSemaphores.empty())
			submitInfo.setSignalSemaphores(signalSemaphores);
		else
			submitInfo.setSignalSemaphores(VK_NULL_HANDLE);

		if (testBitMask(GetRenderPassType(), RenderPassType::RayTracingBit))
			vkCore.GetComputeQueue().submit(submitInfo);
		else
			vkCore.GetGraphicsQueue().submit(submitInfo);
	}

	void VulkanRenderPass::RestartRenderPass()
	{
		const VulkanCore& vkCore = VulkanCore::GetConstInstance();
		if (testBitMask(GetRenderPassType(), RenderPassType::Dynamic))
			m_CommandBuffers[vkCore.GetCurrentFrameIndex()].reset(vk::CommandBufferResetFlags());
		else
			Log<LogLevel::Critical>("RestartRenderPass can not be call on static render pass. This is not allowed.");
	}

	void VulkanRenderPass::DrawImGui(ImDrawData* drawData)
	{
		const vk::CommandBuffer& cmdBuf = m_CommandBuffers[VulkanCore::GetConstInstance().GetCurrentFrameIndex()];
		ImGui_ImplVulkan_RenderDrawData(drawData, cmdBuf);
	}

	void VulkanRenderPass::BindDynamicTexture(const BaseShader& shader, u32 binding, u32 textureID)
	{
		const auto bindPoint = testBitMask(GetRenderPassType(), RenderPassType::RayTracingBit) ? vk::PipelineBindPoint::eRayTracingKHR : vk::PipelineBindPoint::eGraphics;

		(void)binding;
		(void)textureID;
		const auto& vulkanShader = dynamic_cast<const VulkanBaseShader&>(shader);
		if (testBitMask(GetRenderPassType(), RenderPassType::Dynamic))
			m_CommandBuffers[VulkanCore::GetConstInstance().GetCurrentFrameIndex()].bindDescriptorSets(
				bindPoint,
				vulkanShader.GetPipelineLayout(),
				0,
				vulkanShader.GetDescriptorSet(),
				{}
			);
		else
			for (const auto& cmdBuf : m_CommandBuffers)
				cmdBuf.bindDescriptorSets(
					bindPoint,
					vulkanShader.GetPipelineLayout(),
					0,
					vulkanShader.GetDescriptorSet(),
					{}
				);
	}

	uSize VulkanRenderPass::GetNumberOfFramesInFlight() const
	{
		return VulkanCore::GetConstInstance().GetNumberOfFramesInFlight();
	}
}