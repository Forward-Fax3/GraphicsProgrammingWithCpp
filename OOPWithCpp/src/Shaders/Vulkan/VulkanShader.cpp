#include "Application.hpp"
#include "VulkanCore.hpp"
#include "VulkanShader.hpp"
#include "VulkanUniformBuffer.hpp"
#include "Log.hpp"

#include "VulkanTLAS.hpp"


namespace OWC::Graphics
{
	VulkanBaseShader::~VulkanBaseShader()
	{
		const auto& device = VulkanCore::GetConstInstance().GetDevice();

		device.destroyDescriptorPool(m_DescriptorPool);
		device.destroyDescriptorSetLayout(m_DescriptorSetLayout);

		device.destroyPipelineLayout(m_PipelineLayout);
		device.destroyPipeline(m_Pipeline);
	}

	void VulkanBaseShader::BindUniform(u32 binding, const std::shared_ptr<UniformBuffer>& uniformBuffer)
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
//				vulkanUniformBuffer->GetBufferSize()
				vk::WholeSize
			);
			Log<LogLevel::Trace>("vulkanUniformBuffer->GetBufferSize(): {}", vulkanUniformBuffer->GetBufferSize());
			writeDescriptorSets.emplace_back(
				m_DescriptorSets[i],
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

	void VulkanBaseShader::BindTexture(u32 binding, const std::shared_ptr<TextureBuffer>& textureBuffer)
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
			.setImageLayout(vk::ImageLayout::eGeneral)
			.setImageView(vulkanTextureBuffer->GetImageView())
			.setSampler(vulkanTextureBuffer->GetSampler());

		std::vector<vk::WriteDescriptorSet> writeDescriptorSets{};
		writeDescriptorSets.reserve(vkCore.GetNumberOfFramesInFlight());

		for (uSize i = 0; i < vkCore.GetNumberOfFramesInFlight(); i++)
		{
			writeDescriptorSets.emplace_back(
				m_DescriptorSets[i],
				binding,
				0,
				1,
				vk::DescriptorType::eStorageImage,
				&descriptorImageInfo
			);
		}

		device.updateDescriptorSets(writeDescriptorSets, {});
	}

	void VulkanBaseShader::BindDynamicTexture(u32 binding, const std::shared_ptr<DynamicTextureBuffer>& dTextureBuffer)
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
				m_DescriptorSets[i],
				binding,
				0,
				1,
				vk::DescriptorType::eCombinedImageSampler,
				&descriptorImageInfos.back()
			);
		}
		device.updateDescriptorSets(writeDescriptorSets, {});
	}

	VulkanShaderData VulkanBaseShader::ProcessShaderData(const ShaderData& shaderData, std::map<std::vector<u32>*, vk::ShaderModuleCreateInfo>& srcToShaderModulesMap)
	{
		Log<LogLevel::Trace>("VulkanShader::ProcessShaderData: Processing shader data of type {}.", shaderData.ShaderTypeToString());

		VulkanShaderData vulkanShaderData;

		vulkanShaderData.entryPoint = shaderData.entryPoint.empty() ? "main" : shaderData.entryPoint;
		if (shaderData.language == ShaderData::ShaderLanguage::SPIRV) // will always be true for now as the check is done earlier
		{
			auto moduleFound = srcToShaderModulesMap.find(&shaderData.bytecode);

			if (moduleFound == srcToShaderModulesMap.end())
				srcToShaderModulesMap.insert(
					std::make_pair(
						&shaderData.bytecode,
						vk::ShaderModuleCreateInfo(
							vk::ShaderModuleCreateFlags(),
							shaderData.bytecode.size() * sizeof(u32),
							shaderData.bytecode.data()
						)
					)
				);

			vulkanShaderData.moduleKey = &shaderData.bytecode;
		}
		// in the future, add support for other shader languages here (e.g., GLSL to SPIR-V compilation)
		vulkanShaderData.stage = ConvertToVulkanShaderStage(shaderData.type);
		vulkanShaderData.bindingDescriptions =
			std::vector<BindingDescription>(shaderData.descriptorType.begin(), shaderData.descriptorType.end());
		return vulkanShaderData;
	}

	vk::ShaderStageFlagBits VulkanBaseShader::ConvertToVulkanShaderStage(const ShaderType type)
	{
		vk::ShaderStageFlags stage;

		if (testBitMask(type, ShaderType::Vertex))
			stage |= vk::ShaderStageFlagBits::eVertex;
		if (testBitMask(type, ShaderType::Fragment))
			stage |= vk::ShaderStageFlagBits::eFragment;
		if (testBitMask(type, ShaderType::Compute))
			stage |= vk::ShaderStageFlagBits::eCompute;

		if (testBitMask(type, ShaderType::RayGen))
			stage |= vk::ShaderStageFlagBits::eRaygenKHR;
		if (testBitMask(type, ShaderType::RayAnyHit))
			stage |= vk::ShaderStageFlagBits::eAnyHitKHR;
		if (testBitMask(type, ShaderType::RayClosestHit))
			stage |= vk::ShaderStageFlagBits::eClosestHitKHR;
		if (testBitMask(type, ShaderType::RayMiss))
			stage |= vk::ShaderStageFlagBits::eMissKHR;
		if (testBitMask(type, ShaderType::RayIntersect))
			stage |= vk::ShaderStageFlagBits::eIntersectionKHR;

		if (stage == vk::ShaderStageFlags())
			Log<LogLevel::Error>("VulkanShader::ConvertToVulkanShaderStage: Unknown shader type provided: {}!", std::to_underlying(type));

		return static_cast<vk::ShaderStageFlagBits>(static_cast<u32>(stage)); // don't ask
	}

	vk::DescriptorType VulkanBaseShader::ConvertToVulkanDescriptorType(const DescriptorType type)
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
		case DescriptorType::StorageImage:
			return vk::DescriptorType::eStorageImage;
		case DescriptorType::TLAS:
			return vk::DescriptorType::eAccelerationStructureKHR;
		default:
			Log<LogLevel::Error>("VulkanShader::ConvertToVulkanDescriptorType: Unknown descriptor type provided: {}!", std::to_underlying(type));
			return static_cast<vk::DescriptorType>(0); // to silence compiler warning
		}
	}


	VulkanShader::VulkanShader(const std::span<ShaderData>& shaderDatas)
	{
		Log<LogLevel::Trace>("VulkanShader::VulkanShader: Creating Vulkan shader with {} shader stages.", shaderDatas.size());

		if (shaderDatas.empty())
			Log<LogLevel::Error>("VulkanShader::VulkanShader: No shader data provided!");

		std::vector<VulkanShaderData> vulkanShaderDatas;
		vulkanShaderDatas.reserve(shaderDatas.size());

		std::map<std::vector<u32>*, vk::ShaderModuleCreateInfo> srcToShaderModuleMap;

		for (const auto& shaderData : shaderDatas)
		{
			if (shaderData.language != ShaderData::ShaderLanguage::SPIRV)
				Log<LogLevel::Error>("VulkanShader::VulkanShader: Unsupported shader language for Vulkan shader: {}!", std::to_underlying(shaderData.language));

			if (shaderData.type >= ShaderType::RayGen)
				Log<LogLevel::Error>("VulkanShader::VulkanShader: Unsupported shader type for Vulkan graphics pipeline shader: {}! For this shader type please use ray tracing pipeline", shaderData.ShaderTypeToString());

			Log<LogLevel::Debug>("VulkanShader::VulkanShader: Processing {} shader.", shaderData.ShaderTypeToString());

			vulkanShaderDatas.push_back(ProcessShaderData(shaderData, srcToShaderModuleMap));
		}

		CreateVulkanPipeline(vulkanShaderDatas, srcToShaderModuleMap);
	}

	void VulkanShader::CreateVulkanPipeline(const std::span<VulkanShaderData>& vulkanShaderDatas, const std::map<std::vector<u32>*, vk::ShaderModuleCreateInfo>& srcToShaderModulesMap)
	{
		const auto& app = Application::GetConstInstance();

		// Create shader modules and pipeline

		Log<LogLevel::Trace>("VulkanShader::CreateVulkanPipeline: Creating Vulkan pipeline with {} shader stages.", vulkanShaderDatas.size());
		const auto& vkCore = VulkanCore::GetConstInstance();
		const auto& device = vkCore.GetDevice();

		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
		shaderStages.reserve(vulkanShaderDatas.size());

		for (const auto& shaderData : vulkanShaderDatas)
			shaderStages.emplace_back(
				static_cast<vk::PipelineShaderStageCreateFlags>(0),
				shaderData.stage,
				VK_NULL_HANDLE,
				shaderData.entryPoint.c_str(),
				VK_NULL_HANDLE,
				&srcToShaderModulesMap.at(shaderData.moduleKey)
			);

		constexpr std::array dynamicStates = {
			vk::DynamicState::eViewport,
			vk::DynamicState::eScissor
		};

		const vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo = vk::PipelineDynamicStateCreateInfo()
			.setDynamicStates(dynamicStates);

		constexpr vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
		constexpr vk::PipelineInputAssemblyStateCreateInfo inputAssembly = vk::PipelineInputAssemblyStateCreateInfo()
			.setTopology(vk::PrimitiveTopology::eTriangleList)
			.setPrimitiveRestartEnable(vk::False);

		const vk::Viewport viewport = vk::Viewport()
			.setWidth(static_cast<f32>(app.GetPixelWidth()))
			.setHeight(static_cast<f32>(app.GetPixelHeight()))
			.setMinDepth(0.0f)
			.setMaxDepth(1.0f)
			.setX(0.0f)
			.setY(0.0f);
		
		const vk::Rect2D scissor = vk::Rect2D()
			.setOffset({ 0, 0 })
			.setExtent({
				app.GetPixelWidth(),
				app.GetPixelHeight()
			});

		const vk::PipelineViewportStateCreateInfo viewportState = vk::PipelineViewportStateCreateInfo()
			.setViewports(viewport)
			.setScissors(scissor);

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
			.setAttachments(colorBlendAttachment);

		const vk::PipelineRenderingCreateInfo pipelineRenderingInfo = vk::PipelineRenderingCreateInfo()
			.setColorAttachmentFormats(VulkanCore::GetConstInstance().GetSwapchainImageFormat());

		std::vector<vk::DescriptorSetLayoutBinding> bindings;

		for (const auto& shaderData : vulkanShaderDatas)
			for (const auto& [descriptorCount, binding, descriptorType, stageFlags] : shaderData.bindingDescriptions)
				bindings.emplace_back(
					binding,
					ConvertToVulkanDescriptorType(descriptorType),
					descriptorCount,
					ConvertToVulkanShaderStage(stageFlags)
				);

		vk::DescriptorSetLayoutCreateInfo layoutInfo = vk::DescriptorSetLayoutCreateInfo()
			.setBindings(bindings);

		SetDescriptorSetLayout(device.createDescriptorSetLayout(layoutInfo));

		vk::PipelineLayoutCreateInfo pipelineLayoutInfo = vk::PipelineLayoutCreateInfo()
			.setSetLayouts(GetDescriptorSetLayout());

		SetPipelineLayout(device.createPipelineLayout(pipelineLayoutInfo));

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
			.setLayout(GetPipelineLayout())
			.setSubpass(0);

		vk::Pipeline pipeline = vk::Pipeline();

		auto result = device.createGraphicsPipelines(
			nullptr,
			1,
			&pipelineInfo,
			nullptr,
			&pipeline
		);

		if (result != vk::Result::eSuccess)
			Log<LogLevel::Error>("VulkanShader::CreateVulkanPipeline: Failed to create Vulkan graphics pipeline!");
		else
			SetPipeline(pipeline);

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

		SetDescriptorPool(device.createDescriptorPool(poolInfo));
		vk::DescriptorSetAllocateInfo allocInfo = vk::DescriptorSetAllocateInfo()
			.setDescriptorPool(GetDescriptorPool())
			.setSetLayouts(GetDescriptorSetLayout());

		std::vector<vk::DescriptorSet> descriptorSets;
		descriptorSets.reserve(vkCore.GetNumberOfFramesInFlight());

		for (uSize i = 0; i < vkCore.GetNumberOfFramesInFlight(); i++)
			descriptorSets.push_back(device.allocateDescriptorSets(allocInfo).front());

		SetDescriptorSets(descriptorSets);
	}

	VulkanRayTracingShader::VulkanRayTracingShader(const std::span<ShaderData>& shaderDatas)
	{
		Log<LogLevel::Trace>("VulkanShader::VulkanShader: Creating Vulkan shader with {} shader stages.", shaderDatas.size());

		if (shaderDatas.empty())
			Log<LogLevel::Error>("VulkanShader::VulkanShader: No shader data provided!");

		std::vector<VulkanShaderData> vulkanShaderDatas;
		vulkanShaderDatas.reserve(shaderDatas.size());

		std::map<std::vector<u32>*, vk::ShaderModuleCreateInfo> srcToShaderModuleMap;

		for (const auto& shaderData : shaderDatas)
		{
			if (shaderData.language != ShaderData::ShaderLanguage::SPIRV)
				Log<LogLevel::Error>("VulkanShader::VulkanShader: Unsupported shader language for Vulkan shader: {}!", std::to_underlying(shaderData.language));

			if (shaderData.type < ShaderType::RayGen)
				Log<LogLevel::Error>("VulkanRayTracingShader::VulkanRayTracingShader: Unsupported shader type for Vulkan ray tracing pipeline shader: {}! For this shader type please use graphics pipeline", shaderData.ShaderTypeToString());

			Log<LogLevel::Debug>("VulkanShader::VulkanShader: Processing {} shader.", shaderData.ShaderTypeToString());

			vulkanShaderDatas.push_back(ProcessShaderData(shaderData, srcToShaderModuleMap));
		}

		CreateVulkanRayTracingPipeline(vulkanShaderDatas, srcToShaderModuleMap);
		CreateShaderBindingTable(static_cast<u32>(vulkanShaderDatas.size()));
	}

	void VulkanRayTracingShader::BindTLAS(u32 binding, const std::shared_ptr<BaseTLAS>& tlas)
	{
		const auto vulkanTLASBuffer = std::dynamic_pointer_cast<VulkanTLAS>(tlas);

		const auto& vkCore = VulkanCore::GetConstInstance();
		const auto& device = vkCore.GetDevice();

		auto descriptorAccelerationStructureInfo = vk::WriteDescriptorSetAccelerationStructureKHR()
			.setPAccelerationStructures(&vulkanTLASBuffer->GetTLAS())
			.setAccelerationStructureCount(1);

		std::vector<vk::WriteDescriptorSet> writeDescriptorSets{};
		writeDescriptorSets.reserve(vkCore.GetNumberOfFramesInFlight());

		for (uSize i = 0; i < vkCore.GetNumberOfFramesInFlight(); i++)
		{
			writeDescriptorSets.emplace_back(
				GetSpecificDescriptorSet(i),
				binding,
				0,
				1,
				vk::DescriptorType::eAccelerationStructureKHR,
				nullptr,
				nullptr,
				nullptr,
				&descriptorAccelerationStructureInfo
			);
		}

		device.updateDescriptorSets(writeDescriptorSets, {});
	}

	void VulkanRayTracingShader::CreateVulkanRayTracingPipeline(const std::span<VulkanShaderData>& vulkanShaderDatas, const std::map<std::vector<u32>*, vk::ShaderModuleCreateInfo>& srcToShaderModulesMap)
	{
		//const auto& app = Application::GetConstInstance();

		Log<LogLevel::Trace>("VulkanShader::CreateVulkanRayTracingPipeline: Creating Vulkan ray tracing pipeline with {} shader stages.", vulkanShaderDatas.size());

		const auto& vkCore = VulkanCore::GetConstInstance();
		const auto& device = vkCore.GetDevice();

		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
		shaderStages.reserve(vulkanShaderDatas.size());
		std::vector<vk::RayTracingShaderGroupCreateInfoKHR> shaderGroups;
		shaderGroups.reserve(vulkanShaderDatas.size());

		std::vector<vk::DescriptorSetLayoutBinding> bindings;

		for (u32 i = 0; i < vulkanShaderDatas.size(); i++)
		{
			const auto& [entryPoint, moduleKey, stage, bindingDescriptions] = vulkanShaderDatas[i];

			shaderStages.emplace_back(
				static_cast<vk::PipelineShaderStageCreateFlags>(0),
				stage,
				VK_NULL_HANDLE,
				entryPoint.c_str(),
				VK_NULL_HANDLE,
				&srcToShaderModulesMap.at(moduleKey)
			);

			switch (stage)
			{
			case vk::ShaderStageFlagBits::eRaygenKHR:
				shaderGroups.emplace_back(
					vk::RayTracingShaderGroupTypeKHR::eGeneral,
					i
				);
				break;
			case vk::ShaderStageFlagBits::eClosestHitKHR:
				shaderGroups.emplace_back(
					vk::RayTracingShaderGroupTypeKHR::eTrianglesHitGroup,
					VK_SHADER_UNUSED_KHR,
					i
				);
				break;
			case vk::ShaderStageFlagBits::eMissKHR:
				shaderGroups.emplace_back(
					vk::RayTracingShaderGroupTypeKHR::eGeneral,
					i
				);
				break;
			default:
				Log<LogLevel::Error>("VulkanShader::CreateVulkanRayTracingPipeline: Unsupported shader stage for ray tracing pipeline: {}!", vk::to_string(stage));
			}

			for (const auto& [descriptorCount, binding, descriptorType, stageFlags] : bindingDescriptions)
				bindings.emplace_back(
					binding,
					ConvertToVulkanDescriptorType(descriptorType),
					descriptorCount,
					ConvertToVulkanShaderStage(stageFlags)
				);
		}

		const vk::DescriptorSetLayoutCreateInfo layoutInfo = vk::DescriptorSetLayoutCreateInfo()
			.setBindings(bindings);

		SetDescriptorSetLayout(device.createDescriptorSetLayout(layoutInfo));

		vk::PushConstantRange pushConstantRange = vk::PushConstantRange()
			.setOffset(0)
			.setSize(128) // TODO: make this configurable
			.setStageFlags(vk::ShaderStageFlagBits::eRaygenKHR | vk::ShaderStageFlagBits::eClosestHitKHR | vk::ShaderStageFlagBits::eMissKHR);

		const vk::PipelineLayoutCreateInfo pipelineLayoutInfo = vk::PipelineLayoutCreateInfo()
			.setSetLayouts(GetDescriptorSetLayout())
			.setPushConstantRanges(pushConstantRange);

		SetPipelineLayout(device.createPipelineLayout(pipelineLayoutInfo));

		const vk::RayTracingPipelineCreateInfoKHR rayTracingPipelineInfo = vk::RayTracingPipelineCreateInfoKHR()
			.setStages(shaderStages)
			.setGroups(shaderGroups)
			.setMaxPipelineRayRecursionDepth(8) // TODO: make this configurable
			.setLayout(GetPipelineLayout());

		const auto result = device.createRayTracingPipelineKHR(
			{},
			{},
			rayTracingPipelineInfo
		);

		if (result.result != vk::Result::eSuccess)
			Log<LogLevel::Error>("VulkanShader::CreateVulkanRayTracingPipeline: Failed to create Vulkan ray tracing pipeline!");
		else
			SetPipeline(result.value);

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

		SetDescriptorPool(device.createDescriptorPool(poolInfo));
		vk::DescriptorSetAllocateInfo allocInfo = vk::DescriptorSetAllocateInfo()
			.setDescriptorPool(GetDescriptorPool())
			.setSetLayouts(GetDescriptorSetLayout());

		std::vector<vk::DescriptorSet> descriptorSets;
		descriptorSets.reserve(vkCore.GetNumberOfFramesInFlight());

		for (uSize i = 0; i < vkCore.GetNumberOfFramesInFlight(); i++)
			descriptorSets.push_back(device.allocateDescriptorSets(allocInfo).front());

		SetDescriptorSets(descriptorSets);
	}

	void VulkanRayTracingShader::CreateShaderBindingTable(const u32 numberOfShaders)
	{
		const auto& vkCore = VulkanCore::GetConstInstance();
		const auto& device = vkCore.GetDevice();
		const auto& RTProps = vkCore.GetRayTracingPipelineProperties();

		const u32 handleSize = RTProps.shaderGroupHandleSize;
		const u32 handleAlignment = RTProps.shaderGroupHandleAlignment;
		const u32 baseAlignment = RTProps.shaderGroupBaseAlignment;

		const uSize shaderHandleSize = handleSize * numberOfShaders;
		m_ShaderHandles.resize(shaderHandleSize);
		auto result = device.getRayTracingShaderGroupHandlesKHR(GetPipeline(), 0, numberOfShaders, shaderHandleSize, m_ShaderHandles.data());
		if (result != vk::Result::eSuccess)
		{
			Log<LogLevel::Error>("VulkanRayTracingShader::CreateShaderBindingTable: Failed to get ray tracing shader group handles!");
			return;
		}

		// nVidia vk_raytracing_tutorial helper func
		auto alignUp = [](auto value, size_t alignment) noexcept { return ((value + alignment - 1) & ~(alignment - 1)); };
		u32 raygenSize   = alignUp(handleSize, handleAlignment);
		u32 missSize     = alignUp(handleSize, handleAlignment);
		u32 hitSize      = alignUp(handleSize, handleAlignment);
		u32 callableSize = 0;  // Callable shaders not supported yet

		u32 raygenOffset   = 0;
		u32 missOffset     = alignUp(raygenSize, baseAlignment);
		u32 hitOffset      = alignUp(missOffset + missSize, baseAlignment);
		u32 callableOffset = alignUp(hitOffset + hitSize, baseAlignment);

		const uSize bufferSize = callableOffset + callableSize;
		m_SBTBuffer = std::make_unique<VulkanGeneralBuffer>(bufferSize);

		m_SBTBuffer->UpdateBufferData(m_ShaderHandles.data(), handleSize, raygenOffset);
		m_SBTBuffer->UpdateBufferData(m_ShaderHandles.data() + missOffset, handleSize, missOffset);
		m_SBTBuffer->UpdateBufferData(m_ShaderHandles.data() + hitOffset, handleSize, hitOffset);

		m_RayGenShaderSBTEntry
			.setAddress(m_SBTBuffer->GetBufferDeviceAddress() + raygenOffset)
			.setSize(raygenSize)
			.setStride(raygenSize);

		m_HitGroupSBTEntry
			.setAddress(m_SBTBuffer->GetBufferDeviceAddress() + hitOffset)
			.setSize(hitSize)
			.setStride(hitSize);

		m_MissGroupSBTEntry
			.setAddress(m_SBTBuffer->GetBufferDeviceAddress() + missOffset)
			.setSize(missSize)
			.setStride(missSize);

		m_CallableGroupSBTEntry
			.setAddress(m_SBTBuffer->GetBufferDeviceAddress() + callableOffset)
			.setSize(callableSize)
			.setStride(callableSize);
	}
}
