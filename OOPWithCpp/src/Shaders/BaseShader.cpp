#include "BaseShader.hpp"
#include "VulkanShader.hpp"


namespace OWC::Graphics
{
	std::unique_ptr<BaseShader> BaseShader::CreateShader(const std::span<ShaderData>& shaderDatas)
	{
		// setup so that in future other graphics APIs can be supported
		return std::make_unique<VulkanShader>(shaderDatas);
	}
}
