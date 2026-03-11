#pragma once
#include "BaseShader.hpp"

#include "VulkanCore.hpp"

#include <string>
#include <vector>
#include <span>

#include <vulkan/vulkan.hpp>


namespace OWC::Graphics
{
	class VulkanShader : public BaseShader
	{
	private:
		struct VulkanShaderData
		{
			std::string entryPoint;
			std::vector<u32> bytecode;
			vk::ShaderStageFlagBits stage;
			std::vector<BindingDiscription> bindingDescriptions;
		};

	public:
		explicit VulkanShader(const std::span<ShaderData>& shaderDatas);
		~VulkanShader() override;
		VulkanShader(VulkanShader&) = delete;
		VulkanShader& operator=(VulkanShader&) = delete;
		VulkanShader(VulkanShader&&) noexcept = delete;
		VulkanShader& operator=(VulkanShader&&) noexcept = delete;

		void BindUniform(u32 binding, const std::shared_ptr<UniformBuffer>& uniformBuffer) override;
		void BindTexture(u32 binding, const std::shared_ptr<TextureBuffer>& textureBuffer) override;
		void BindDynamicTexture(u32 binding, const std::shared_ptr<DynamicTextureBuffer>& dTextureBuffer) override;

		[[nodiscard]] vk::Pipeline GetPipeline() const { return m_Pipeline; }
		[[nodiscard]] vk::PipelineLayout GetPipelineLayout() const { return m_PipelineLayout; }
		[[nodiscard]] vk::DescriptorSetLayout GetDescriptorSetLayout() const { return m_DescriptorSetLayout; }
		[[nodiscard]] vk::DescriptorSet GetDescriptorSet() const { return m_DescriptorSet[VulkanCore::GetConstInstance().GetCurrentFrameIndex()]; }

	private:
		void CreateVulkanPipeline(const std::span<VulkanShaderData>& vulkanShaderDatas);

		[[nodiscard]] static VulkanShaderData ProcessShaderData(const ShaderData& shaderData);
		[[nodiscard]] static vk::ShaderStageFlagBits ConvertToVulkanShaderStage(ShaderType type);
		[[nodiscard]] static vk::DescriptorType ConvertToVulkanDescriptorType(DescriptorType type);

	private:
		// store pipeline and shader module handles here
		vk::Pipeline m_Pipeline = vk::Pipeline();
		vk::PipelineLayout m_PipelineLayout = vk::PipelineLayout();
		vk::DescriptorSetLayout m_DescriptorSetLayout = vk::DescriptorSetLayout();
		vk::DescriptorPool m_DescriptorPool = vk::DescriptorPool();
		std::vector<vk::DescriptorSet> m_DescriptorSet = {};
		std::vector<vk::ShaderModule> m_ShaderModules = {};
	};
}