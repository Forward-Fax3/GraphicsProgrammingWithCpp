#include "Application.hpp"
#include "VulkanCore.hpp"
#include "VulkanShader.hpp"
#include "VulkanUniformBuffer.hpp"
#include "Log.hpp"


namespace OWC::Graphics
{
	VulkanShader::VulkanShader(const std::span<ShaderData>& shaderDatas)
	{
		if (shaderDatas.empty())
			Log<LogLevel::Error>("VulkanShader::VulkanShader: No shader data provided!");

		std::vector<VulkanShaderData> vulkanShaderDatas;
		vulkanShaderDatas.reserve(shaderDatas.size());

		for (const auto& shaderData : shaderDatas)
		{
			if (shaderData.language != ShaderData::ShaderLanguage::SPIRV)
				Log<LogLevel::Error>("VulkanShader::VulkanShader: Unsupported shader language for Vulkan shader: {}!", std::to_underlying(shaderData.language));

			Log<LogLevel::Debug>("VulkanShader::VulkanShader: Processing {} shader.", shaderData.ShaderTypeToString());

			vulkanShaderDatas.push_back(ProcessShaderData(shaderData));
		}

		CreateVulkanPipeline(vulkanShaderDatas);
	}

	VulkanShader::~VulkanShader()
	{
		VulkanCore::GetInstance().GetDevice().destroyDescriptorPool(m_DescriptorPool);
		VulkanCore::GetInstance().GetDevice().destroyDescriptorSetLayout(m_DescriptorSetLayout);

		VulkanCore::GetInstance().GetDevice().destroyPipeline(m_Pipeline);
		VulkanCore::GetInstance().GetDevice().destroyPipelineLayout(m_PipelineLayout);

		for (const auto& shaderModule : m_ShaderModules)
			VulkanCore::GetInstance().GetDevice().destroyShaderModule(shaderModule);
	}

	void VulkanShader::BindUniform(u32 binding, const std::shared_ptr<UniformBuffer>& uniformBuffer)
	{
		const auto vulkanUniformBuffer = std::dynamic_pointer_cast<VulkanUniformBuffer>(uniformBuffer);

		const auto& vkCore = VulkanCore::GetConstInstance();
		const auto& device = vkCore.GetDevice();

		std::vector<vk::WriteDescriptorSet> writeDescriptorSets{};
		writeDescriptorSets.reserve(vulkanUniformBuffer->GetBuffers().size());

		std::vector<vk::DescriptorBufferInfo> descriptorBufferInfos{}; // keep alive until updateDescriptorSets is called
		descriptorBufferInfos.reserve(vulkanUniformBuffer->GetBuffers().size());
		
		for (uSize i = 0; i < vulkanUniformBuffer->GetBuffers().size(); i++)
		{
			descriptorBufferInfos.emplace_back(
				vulkanUniformBuffer->GetBuffers()[i],
				0,
				vulkanUniformBuffer->GetBufferSize()
			);
			writeDescriptorSets.emplace_back(
				m_DescriptorSet[i],
				binding,
				0,
				1,
				vk::DescriptorType::eUniformBuffer,
				nullptr,
				&descriptorBufferInfos.back()
			);
		}

		device.updateDescriptorSets(writeDescriptorSets, {});
	}

	void VulkanShader::BindTexture(u32 binding, const std::shared_ptr<TextureBuffer>& textureBuffer)
	{
		const auto vulkanTextureBuffer = std::dynamic_pointer_cast<VulkanTextureBuffer>(textureBuffer);
		if (!vulkanTextureBuffer)
		{
			Log<LogLevel::Error>("VulkanShader::BindTexture: Invalid VulkanTextureBuffer pointer.");
			return;
		}

		const auto& vkCore = VulkanCore::GetConstInstance();
		const auto& device = vkCore.GetDevice();

		vk::DescriptorImageInfo descriptorImageInfo = vk::DescriptorImageInfo()
			.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
			.setImageView(vulkanTextureBuffer->GetImageView())
			.setSampler(vulkanTextureBuffer->GetSampler());

		std::vector<vk::WriteDescriptorSet> writeDescriptorSets{};
		writeDescriptorSets.reserve(vkCore.GetNumberOfFramesInFlight());

		for (uSize i = 0; i < vkCore.GetNumberOfFramesInFlight(); i++)
		{
			writeDescriptorSets.emplace_back(
				m_DescriptorSet[i],
				binding,
				0,
				1,
				vk::DescriptorType::eCombinedImageSampler,
				&descriptorImageInfo
			);
		}

		device.updateDescriptorSets(writeDescriptorSets, {});
	}

	void VulkanShader::BindDynamicTexture(u32 binding, const std::shared_ptr<DynamicTextureBuffer>& dTextureBuffer)
	{
		const auto vulkanDynamicTextureBuffer = std::dynamic_pointer_cast<VulkanDynamicTextureBuffer>(dTextureBuffer);
		if (!vulkanDynamicTextureBuffer)
		{
			Log<LogLevel::Error>("VulkanShader::BindDynamicTexture: Invalid VulkanDynamicTextureBuffer pointer.");
			return;
		}

		const auto& vkCore = VulkanCore::GetConstInstance();
		const auto& device = vkCore.GetDevice();
		std::vector<vk::WriteDescriptorSet> writeDescriptorSets{};
		writeDescriptorSets.reserve(vkCore.GetNumberOfFramesInFlight());

		std::vector<vk::DescriptorImageInfo> descriptorImageInfos{}; // keep alive until updateDescriptorSets is called
		descriptorImageInfos.reserve(vkCore.GetNumberOfFramesInFlight());

		for (uSize i = 0; i < vkCore.GetNumberOfFramesInFlight(); i++)
		{
			descriptorImageInfos.emplace_back(
				vulkanDynamicTextureBuffer->GetSampler(),
				vulkanDynamicTextureBuffer->GetImageViews()[i],
				vk::ImageLayout::eShaderReadOnlyOptimal
			);

			writeDescriptorSets.emplace_back(
				m_DescriptorSet[i],
				binding,
				0,
				1,
				vk::DescriptorType::eCombinedImageSampler,
				&descriptorImageInfos.back()
			);
		}
		device.updateDescriptorSets(writeDescriptorSets, {});
	}

	void VulkanShader::CreateVulkanPipeline(const std::span<VulkanShaderData>& vulkanShaderDatas)
	{
		// Create shader modules and pipeline

		Log<LogLevel::Trace>("VulkanShader::CreateVulkanPipeline: Creating Vulkan pipeline with {} shader stages.", vulkanShaderDatas.size());
		const auto& vkCore = VulkanCore::GetConstInstance();
		const auto& device = vkCore.GetDevice();

		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
		m_ShaderModules.reserve(vulkanShaderDatas.size());
		shaderStages.reserve(vulkanShaderDatas.size());

		for (const auto& shaderData : vulkanShaderDatas)
		{
			const vk::ShaderModuleCreateInfo shaderModuleCreateInfo = vk::ShaderModuleCreateInfo()
				.setCodeSize(shaderData.bytecode.size() * sizeof(u32))
				.setPCode(shaderData.bytecode.data());

			m_ShaderModules.emplace_back(device.createShaderModule(shaderModuleCreateInfo));

			shaderStages.emplace_back(
				static_cast<vk::PipelineShaderStageCreateFlags>(0),
				shaderData.stage,
				m_ShaderModules.back(),
				shaderData.entryPoint.c_str()
			);
		}

		constexpr std::array<vk::DynamicState, 2> dynamicStates = {
			vk::DynamicState::eViewport,
			vk::DynamicState::eScissor
		};

		const vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo = vk::PipelineDynamicStateCreateInfo()
			.setDynamicStateCount(static_cast<u32>(dynamicStates.size()))
			.setPDynamicStates(dynamicStates.data());

		constexpr vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
		constexpr vk::PipelineInputAssemblyStateCreateInfo inputAssembly = vk::PipelineInputAssemblyStateCreateInfo()
			.setTopology(vk::PrimitiveTopology::eTriangleList)
			.setPrimitiveRestartEnable(vk::False);

		const vk::Viewport viewport = vk::Viewport()
			.setHeight(static_cast<f32>(Application::GetConstInstance().GetWindow().GetHeight()))
			.setWidth(static_cast<f32>(Application::GetConstInstance().GetWindow().GetWidth()))
			.setMinDepth(0.0f)
			.setMaxDepth(1.0f)
			.setX(0.0f)
			.setY(0.0f);
		
		const vk::Rect2D scissor = vk::Rect2D()
			.setOffset({ 0, 0 })
			.setExtent({
				static_cast<u32>(Application::GetConstInstance().GetWindowWidth()),
				static_cast<u32>(Application::GetConstInstance().GetWindowHeight())
			});

		const vk::PipelineViewportStateCreateInfo viewportState = vk::PipelineViewportStateCreateInfo()
			.setViewportCount(1)
			.setPViewports(&viewport)
			.setScissorCount(1)
			.setPScissors(&scissor);

		constexpr vk::PipelineRasterizationStateCreateInfo rasterizer = vk::PipelineRasterizationStateCreateInfo()
			.setPolygonMode(vk::PolygonMode::eFill)
			.setLineWidth(1.0f);

		constexpr vk::PipelineMultisampleStateCreateInfo multisampling = vk::PipelineMultisampleStateCreateInfo()
			.setSampleShadingEnable(vk::False)
			.setRasterizationSamples(vk::SampleCountFlagBits::e1);

		constexpr vk::PipelineColorBlendAttachmentState colorBlendAttachment = vk::PipelineColorBlendAttachmentState()
			.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
				vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA)
			.setBlendEnable(vk::True)
			.setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
			.setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
			.setColorBlendOp(vk::BlendOp::eAdd)
			.setSrcAlphaBlendFactor(vk::BlendFactor::eOne);

		const vk::PipelineColorBlendStateCreateInfo colorBlending = vk::PipelineColorBlendStateCreateInfo()
			.setLogicOpEnable(vk::False)
			.setAttachmentCount(1)
			.setPAttachments(&colorBlendAttachment);

		const vk::PipelineRenderingCreateInfo pipelineRenderingInfo = vk::PipelineRenderingCreateInfo()
			.setColorAttachmentCount(1)
			.setPColorAttachmentFormats(&VulkanCore::GetConstInstance().GetSwapchainImageFormat());

		std::vector<vk::DescriptorSetLayoutBinding> bindings;

		for (const auto& shaderData : vulkanShaderDatas)
			for (const auto& bindingDescription : shaderData.bindingDescriptions)
				bindings.emplace_back(
					bindingDescription.binding,
					ConvertToVulkanDescriptorType(bindingDescription.descriptorType),
					bindingDescription.descriptorCount,
					ConvertToVulkanShaderStage(bindingDescription.stageFlags)
				);

		vk::DescriptorSetLayoutCreateInfo layoutInfo = vk::DescriptorSetLayoutCreateInfo()
			.setBindings(bindings);

		m_DescriptorSetLayout = device.createDescriptorSetLayout(layoutInfo);

		vk::PipelineLayoutCreateInfo pipelineLayoutInfo = vk::PipelineLayoutCreateInfo()
			.setSetLayouts(m_DescriptorSetLayout);

		m_PipelineLayout = device.createPipelineLayout(pipelineLayoutInfo);

		vk::GraphicsPipelineCreateInfo pipelineInfo = vk::GraphicsPipelineCreateInfo()
			.setPNext(&pipelineRenderingInfo)
			.setStages(shaderStages)
			.setPVertexInputState(&vertexInputInfo)
			.setPInputAssemblyState(&inputAssembly)
			.setPViewportState(&viewportState)
			.setPRasterizationState(&rasterizer)
			.setPMultisampleState(&multisampling)
			.setPColorBlendState(&colorBlending)
			.setPDynamicState(&dynamicStateCreateInfo)
			.setLayout(m_PipelineLayout)
			.setSubpass(0);

		auto result = device.createGraphicsPipelines(
			nullptr,
			1,
			&pipelineInfo,
			nullptr,
			&m_Pipeline
		);

		if (result != vk::Result::eSuccess)
			Log<LogLevel::Error>("VulkanShader::CreateVulkanPipeline: Failed to create Vulkan graphics pipeline!");

		// build Descriptor Pool
		// TODO: make this dynamic based on actual usage

		std::vector<vk::DescriptorPoolSize> poolSize;

		for (const auto& shaderData : vulkanShaderDatas)
		{
			for (const auto& bindingDescription : shaderData.bindingDescriptions)
			{
				vk::DescriptorType descriptorType = ConvertToVulkanDescriptorType(bindingDescription.descriptorType);

				auto it = std::ranges::find_if(poolSize, [descriptorType](const vk::DescriptorPoolSize& size)
					{
						return size.type == descriptorType;
					});

				if (it != poolSize.end())
				{
					it->descriptorCount += bindingDescription.descriptorCount * static_cast<u32>(vkCore.GetNumberOfFramesInFlight());
				}
				else
				{
					poolSize.emplace_back(
						descriptorType,
						bindingDescription.descriptorCount * static_cast<u32>(vkCore.GetNumberOfFramesInFlight())
					);
				}
			}
		}

		vk::DescriptorPoolCreateInfo poolInfo = vk::DescriptorPoolCreateInfo()
			.setPoolSizes(poolSize)
			.setMaxSets(static_cast<u32>(vkCore.GetNumberOfFramesInFlight()));

		m_DescriptorPool = device.createDescriptorPool(poolInfo);
		vk::DescriptorSetAllocateInfo allocInfo = vk::DescriptorSetAllocateInfo()
			.setDescriptorPool(m_DescriptorPool)
			.setSetLayouts(m_DescriptorSetLayout);
		for (uSize i = 0; i < vkCore.GetNumberOfFramesInFlight(); i++)
		{
			m_DescriptorSet.push_back(device.allocateDescriptorSets(allocInfo).front());
		}
	}

	VulkanShader::VulkanShaderData VulkanShader::ProcessShaderData(const ShaderData& shaderData)
	{
		Log<LogLevel::Trace>("VulkanShader::ProcessShaderData: Processing shader data of type {}.", shaderData.ShaderTypeToString());

		VulkanShaderData vulkanShaderData;

		vulkanShaderData.entryPoint = "main";
		if (shaderData.language == ShaderData::ShaderLanguage::SPIRV) // will always be true for now as the check is done earlier
			vulkanShaderData.bytecode = shaderData.bytecode;
		// in the future, add support for other shader languages here (e.g., GLSL to SPIR-V compilation)
		vulkanShaderData.stage = ConvertToVulkanShaderStage(shaderData.type);
		vulkanShaderData.bindingDescriptions = std::vector<BindingDiscription>(shaderData.descriptorType.begin(), shaderData.descriptorType.end());
		return vulkanShaderData;
	}

	vk::ShaderStageFlagBits VulkanShader::ConvertToVulkanShaderStage(ShaderType type)
	{
		vk::ShaderStageFlags stage;

		if (type == ShaderType::Vertex)
			stage |= vk::ShaderStageFlagBits::eVertex;
		if (type == ShaderType::Fragment)
			stage |= vk::ShaderStageFlagBits::eFragment;
		if (type == ShaderType::Compute)
			stage |= vk::ShaderStageFlagBits::eCompute;

		if (stage == vk::ShaderStageFlags())
			Log<LogLevel::Error>("VulkanShader::ConvertToVulkanShaderStage: Unknown shader type provided: {}!", std::to_underlying(type));

		return static_cast<vk::ShaderStageFlagBits>(static_cast<u32>(stage)); // don't ask
	}

	vk::DescriptorType VulkanShader::ConvertToVulkanDescriptorType(DescriptorType type)
	{
		switch (type)
		{
		case DescriptorType::UniformBuffer:
			return vk::DescriptorType::eUniformBuffer;
		case DescriptorType::Sampler:
			return vk::DescriptorType::eSampler;
		case DescriptorType::CombinedImageSampler:
			return vk::DescriptorType::eCombinedImageSampler;
		case DescriptorType::StorageBuffer:
			return vk::DescriptorType::eStorageBuffer;
		default:
			Log<LogLevel::Error>("VulkanShader::ConvertToVulkanDescriptorType: Unknown descriptor type provided: {}!", std::to_underlying(type));
			return static_cast<vk::DescriptorType>(0); // to silence compiler warning
		}
	}
}