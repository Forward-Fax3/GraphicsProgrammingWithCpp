#include "Application.hpp"
#include "VulkanCore.hpp"
#include "VulkanShader.hpp"
#include "Log.hpp"


namespace OWC::Graphics
{
	VulkanShader::VulkanShader(const std::vector<ShaderData>& shaderDatas)
	{
		if (shaderDatas.empty())
			Log<LogLevel::Error>("VulkanShader::VulkanShader: No shader data provided!");

		std::vector<VulkanShaderData> vulkanShaderDatas;
		vulkanShaderDatas.reserve(shaderDatas.size());

		for (const auto& shaderData : shaderDatas)
		{
			if (shaderData.language != ShaderData::ShaderLanguage::SPIRV)
			{
				Log<LogLevel::Error>("VulkanShader::VulkanShader: Unsupported shader language for Vulkan shader: {}!", std::to_underlying(shaderData.language));
			}
			// Here we would normally process the SPIR-V bytecode and create Vulkan shader modules.
			// For now, we just log the shader type being processed.
			Log<LogLevel::Debug>("VulkanShader::VulkanShader: Processing {} shader.", shaderData.ShaderTypeToString());

			vulkanShaderDatas.push_back(ProcessShaderData(shaderData));
		}

		CreateVulkanPipeline(vulkanShaderDatas);
	}

	VulkanShader::~VulkanShader()
	{
		VulkanCore::GetInstance().GetDevice().destroyPipeline(m_Pipeline);
		VulkanCore::GetInstance().GetDevice().destroyPipelineLayout(m_PipelineLayout);

		for (const auto& shaderModule : m_ShaderModules)
			VulkanCore::GetInstance().GetDevice().destroyShaderModule(shaderModule);
	}

	void VulkanShader::CreateVulkanPipeline(const std::vector<VulkanShaderData>& vulkanShaderDatas)
	{
		Log<LogLevel::Trace>("VulkanShader::CreateVulkanPipeline: Creating Vulkan pipeline with {} shader stages.", vulkanShaderDatas.size());

		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
		shaderStages.reserve(vulkanShaderDatas.size());
		m_ShaderModules.reserve(vulkanShaderDatas.size());

		for (const auto& shaderData : vulkanShaderDatas)
		{
			const vk::ShaderModuleCreateInfo shaderModuleCreateInfo = vk::ShaderModuleCreateInfo()
				.setCodeSize(shaderData.bytecode.size() * sizeof(uint32_t))
				.setPCode(shaderData.bytecode.data());

			m_ShaderModules.emplace_back(VulkanCore::GetInstance().GetDevice().createShaderModule(shaderModuleCreateInfo));

			const vk::PipelineShaderStageCreateInfo shaderStageInfo = vk::PipelineShaderStageCreateInfo()
				.setStage(shaderData.stage)
				.setModule(m_ShaderModules.back())
				.setPName(shaderData.entryPoint.c_str());

			shaderStages.push_back(shaderStageInfo);
		}

		constexpr std::array<vk::DynamicState, 2> dynamicStates = {
			vk::DynamicState::eViewport,
			vk::DynamicState::eScissor
		};

		const vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo = vk::PipelineDynamicStateCreateInfo()
			.setDynamicStateCount(static_cast<uint32_t>(dynamicStates.size()))
			.setPDynamicStates(dynamicStates.data());

		constexpr vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
		constexpr vk::PipelineInputAssemblyStateCreateInfo inputAssembly = vk::PipelineInputAssemblyStateCreateInfo()
			.setTopology(vk::PrimitiveTopology::eTriangleList)
			.setPrimitiveRestartEnable(vk::False);

		const vk::Viewport viewport = vk::Viewport()
			.setHeight(static_cast<float>(Application::GetConstInstance().GetWindow().GetHeight()))
			.setWidth(static_cast<float>(Application::GetConstInstance().GetWindow().GetWidth()))
			.setMinDepth(0.0f)
			.setMaxDepth(1.0f)
			.setX(0.0f)
			.setY(0.0f);
		
		const vk::Rect2D scissor = vk::Rect2D()
			.setOffset({ 0, 0 })
			.setExtent({
				static_cast<uint32_t>(Application::GetConstInstance().GetWindowWidth()),
				static_cast<uint32_t>(Application::GetConstInstance().GetWindowHeight())
			});

		const vk::PipelineViewportStateCreateInfo viewportState = vk::PipelineViewportStateCreateInfo()
			.setViewportCount(1)
			.setPViewports(&viewport)
			.setScissorCount(1)
			.setPScissors(&scissor);

		constexpr vk::PipelineRasterizationStateCreateInfo rasterizer = vk::PipelineRasterizationStateCreateInfo()
			.setDepthClampEnable(vk::False)
			.setRasterizerDiscardEnable(vk::False)
			.setPolygonMode(vk::PolygonMode::eFill)
			.setLineWidth(1.0f)
			.setCullMode(vk::CullModeFlagBits::eNone) // not needed for now
//			.setFrontFace(vk::FrontFace::eClockwise)
			.setDepthBiasEnable(vk::False);

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

		vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
		m_PipelineLayout = VulkanCore::GetInstance().GetDevice().createPipelineLayout(pipelineLayoutInfo);

		vk::GraphicsPipelineCreateInfo pipelineInfo = vk::GraphicsPipelineCreateInfo()
			.setPNext(&pipelineRenderingInfo)
			.setStageCount(static_cast<uint32_t>(shaderStages.size()))
			.setPStages(shaderStages.data())
			.setPVertexInputState(&vertexInputInfo)
			.setPInputAssemblyState(&inputAssembly)
			.setPViewportState(&viewportState)
			.setPRasterizationState(&rasterizer)
			.setPMultisampleState(&multisampling)
			.setPColorBlendState(&colorBlending)
			.setPDynamicState(&dynamicStateCreateInfo)
			.setLayout(m_PipelineLayout)
//			.setRenderPass(VulkanCore::GetInstance().GetRenderPass())
			.setSubpass(0);

		auto result = VulkanCore::GetInstance().GetDevice().createGraphicsPipelines(
			nullptr,
			1,
			&pipelineInfo,
			nullptr,
			&m_Pipeline
		);

		if (result != vk::Result::eSuccess)
			Log<LogLevel::Error>("VulkanShader::CreateVulkanPipeline: Failed to create Vulkan graphics pipeline!");
	}

	VulkanShader::VulkanShaderData VulkanShader::ProcessShaderData(const ShaderData& shaderData)
	{
		Log<LogLevel::Trace>("VulkanShader::ProcessShaderData: Processing shader data of type {}.", shaderData.ShaderTypeToString());

		VulkanShaderData vulkanShaderData;

		vulkanShaderData.entryPoint = "main"; // Example entry point naming
		if (shaderData.language == ShaderData::ShaderLanguage::SPIRV)
		{
			vulkanShaderData.bytecode = shaderData.bytecode;
		}
		vulkanShaderData.stage = ConvertToVulkanShaderStage(shaderData.type);
		return vulkanShaderData;
	}

	vk::ShaderStageFlagBits VulkanShader::ConvertToVulkanShaderStage(ShaderData::ShaderType type)
	{
		switch (type)
		{
		using enum OWC::Graphics::ShaderData::ShaderType;
		using enum vk::ShaderStageFlagBits;
		case Vertex:
			return eVertex;
		case Fragment:
			return eFragment;
		case Compute:
			return eCompute;
		default:
			Log<LogLevel::Error>("VulkanShader::ConvertToVulkanShaderStage: Unknown shader type {}!", std::to_underlying(type));
			return eVertex; // Default return to avoid compiler warning
		}
	}
}
