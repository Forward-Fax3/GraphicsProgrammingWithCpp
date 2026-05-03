#pragma once
#include "VulkanCore.hpp"
#include "VulkanUniformBuffer.hpp"

#include <string>
#include <vector>
#include <span>

#include <vulkan/vulkan.hpp>

#include "BaseShader.hpp"


namespace OWC::Graphics
{
	struct VulkanShaderData
	{
		std::string entryPoint;
		std::vector<u32>* moduleKey = nullptr;
		vk::ShaderStageFlagBits stage;
		std::vector<BindingDescription> bindingDescriptions;
	};

	class VulkanBaseShader : public BaseShader
	{
	public:
		VulkanBaseShader() = default;
		~VulkanBaseShader() override;
		VulkanBaseShader(VulkanBaseShader&) = delete;
		VulkanBaseShader& operator=(VulkanBaseShader&) = delete;
		VulkanBaseShader(VulkanBaseShader&&) noexcept = delete;
		VulkanBaseShader& operator=(VulkanBaseShader&&) noexcept = delete;

		void BindUniform(u32 binding, const std::shared_ptr<UniformBuffer>& uniformBuffer) override;
		void BindTexture(u32 binding, const std::shared_ptr<TextureBuffer>& textureBuffer) override;
		void BindDynamicTexture(u32 binding, const std::shared_ptr<DynamicTextureBuffer>& dTextureBuffer) override;

		[[nodiscard]] OWC_FORCE_INLINE vk::Pipeline GetPipeline() const { return m_Pipeline; }
		[[nodiscard]] OWC_FORCE_INLINE vk::PipelineLayout GetPipelineLayout() const { return m_PipelineLayout; }
		[[nodiscard]] OWC_FORCE_INLINE vk::ArrayProxyNoTemporaries<const vk::DescriptorSetLayout> GetDescriptorSetLayout() const { return m_DescriptorSetLayout; }
		[[nodiscard]] OWC_FORCE_INLINE vk::DescriptorSet GetDescriptorSet() const { return m_DescriptorSets[VulkanCore::GetConstInstance().GetCurrentFrameIndex()]; }
		[[nodiscard]] OWC_FORCE_INLINE vk::DescriptorSet GetSpecificDescriptorSet(const uSize i) const { return m_DescriptorSets[i]; }

	protected:
		[[nodiscard]] OWC_FORCE_INLINE vk::DescriptorPool GetDescriptorPool() const { return m_DescriptorPool; }

		void OWC_FORCE_INLINE SetPipeline(const vk::Pipeline pipeline) { m_Pipeline = pipeline; }
		void OWC_FORCE_INLINE SetPipelineLayout(const vk::PipelineLayout pipelineLayout) { m_PipelineLayout = pipelineLayout; }
		void OWC_FORCE_INLINE SetDescriptorSetLayout(const vk::DescriptorSetLayout descriptorSetLayout) { m_DescriptorSetLayout = descriptorSetLayout; }
		void OWC_FORCE_INLINE SetDescriptorPool(const vk::DescriptorPool descriptorPool) { m_DescriptorPool = descriptorPool; }
		void OWC_FORCE_INLINE SetDescriptorSets(const std::vector<vk::DescriptorSet>& descriptorSet) { m_DescriptorSets = descriptorSet; }

		[[nodiscard]] static VulkanShaderData ProcessShaderData(const ShaderData& shaderData, std::map<std::vector<u32>*, vk::ShaderModuleCreateInfo>& srcToShaderModulesMap);
		[[nodiscard]] static vk::ShaderStageFlagBits ConvertToVulkanShaderStage(ShaderType type);
		[[nodiscard]] static vk::DescriptorType ConvertToVulkanDescriptorType(DescriptorType type);

	private:
		vk::Pipeline m_Pipeline = vk::Pipeline();
		vk::PipelineLayout m_PipelineLayout = vk::PipelineLayout();
		vk::DescriptorSetLayout m_DescriptorSetLayout = vk::DescriptorSetLayout();
		vk::DescriptorPool m_DescriptorPool = vk::DescriptorPool();
		std::vector<vk::DescriptorSet> m_DescriptorSets = {};
	};

	class VulkanShader : public VulkanBaseShader
	{
	public:
		explicit VulkanShader(const std::span<ShaderData>& shaderDatas);
		VulkanShader(VulkanShader&) = delete;
		VulkanShader& operator=(VulkanShader&) = delete;
		VulkanShader(VulkanShader&&) noexcept = delete;
		VulkanShader& operator=(VulkanShader&&) noexcept = delete;

		void BindTLAS(u32 binding, const std::shared_ptr<BaseTLAS>& tlasBuffer) override { Log<LogLevel::Error>("VulkanShader::BindTLAS: TLAS can only be bound to ray tracing shaders! For this shader type please use VulkanRayTracingShader."); }

	private:
		void CreateVulkanPipeline(const std::span<VulkanShaderData>& vulkanShaderDatas, const std::map<std::vector<u32>*, vk::ShaderModuleCreateInfo>& srcToShaderModulesMap);
	};

	class VulkanRayTracingShader : public VulkanBaseShader
	{
	public:
		explicit VulkanRayTracingShader(const std::span<ShaderData>& shaderDatas);

		VulkanRayTracingShader(VulkanRayTracingShader&) = delete;
		VulkanRayTracingShader& operator=(VulkanRayTracingShader&) = delete;
		VulkanRayTracingShader(VulkanRayTracingShader&&) noexcept = delete;
		VulkanRayTracingShader& operator=(VulkanRayTracingShader&&) noexcept = delete;

		[[nodiscard]] OWC_FORCE_INLINE const vk::StridedDeviceAddressRangeKHR& GetRayGenShaderSBTEntry() const { return m_RayGenShaderSBTEntry; }
		[[nodiscard]] OWC_FORCE_INLINE const vk::StridedDeviceAddressRangeKHR& GetHitGroupSBTEntry() const { return m_HitGroupSBTEntry; }
		[[nodiscard]] OWC_FORCE_INLINE const vk::StridedDeviceAddressRangeKHR& GetMissGroupSBTEntry() const { return m_MissGroupSBTEntry; }
		[[nodiscard]] OWC_FORCE_INLINE const vk::StridedDeviceAddressRangeKHR& GetCallableGroupSBTEntry() const { return m_CallableGroupSBTEntry; }

		void BindTLAS(u32 binding, const std::shared_ptr<BaseTLAS>& tlasBuffer) override;

	private:
		void CreateVulkanRayTracingPipeline(const std::span<VulkanShaderData>& vulkanShaderDatas, const std::map<std::vector<u32>*, vk::ShaderModuleCreateInfo>& srcToShaderModulesMap);
		void CreateShaderBindingTable(u32 numberOfShaders);

	private:
		vk::StridedDeviceAddressRangeKHR m_RayGenShaderSBTEntry = {};
		vk::StridedDeviceAddressRangeKHR m_HitGroupSBTEntry = {};
		vk::StridedDeviceAddressRangeKHR m_MissGroupSBTEntry = {};
		vk::StridedDeviceAddressRangeKHR m_CallableGroupSBTEntry = {}; // This won't be implemented yet
		std::unique_ptr<VulkanGeneralBuffer> m_SBTBuffer;
		std::vector<u8> m_ShaderHandles;
	};
}
