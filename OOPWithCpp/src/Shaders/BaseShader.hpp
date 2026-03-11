#pragma once
#include "UniformBuffer.hpp"

#include <string>
#include <vector>
#include <span>
#include <memory>


namespace OWC::Graphics
{
	enum class ShaderType : u8
	{
		Vertex = 1<<0,
		Fragment = 1<<1,
		Compute = 1<<2
	};

	enum class DescriptorType : u8
	{
		UniformBuffer,
		Sampler,
		CombinedImageSampler,
		StorageBuffer
	};

	struct BindingDiscription
	{
		u32 descriptorCount;
		u32 binding;
		DescriptorType descriptorType;
		ShaderType stageFlags;
	};

	struct ShaderData
	{
		enum class ShaderLanguage : u8
		{ // for now only SPIR-V is supported
//			GLSL,
//			HLSL,
//			Slang,
			SPIRV,

			SPV = SPIRV
		};

		OWC_FORCE_INLINE std::string ShaderTypeToString() const
		{
			switch (type)
			{
			using enum OWC::Graphics::ShaderType;
			case Vertex:
				return "Vertex";
			case Fragment:
				return "Fragment";
			case Compute:
				return "Compute";
			default:
				return "Unknown";
			}
		}

//		std::string source;
		std::vector<u32> bytecode; // for SPIR-V
		
		ShaderType type;
		ShaderLanguage language;
		std::span<BindingDiscription> descriptorType;
	};

	class BaseShader
	{
	public:
		BaseShader() = default;
		virtual ~BaseShader() = default;
		BaseShader(BaseShader&) = default;
		BaseShader& operator=(BaseShader&) = default;
		BaseShader(BaseShader&&) noexcept = default;
		BaseShader& operator=(BaseShader&&) noexcept = default;

		virtual void BindUniform(u32 binding, const std::shared_ptr<UniformBuffer>& uniformBuffer) = 0;
		virtual void BindTexture(u32 binding, const std::shared_ptr<TextureBuffer>& textureBuffer) = 0;
		virtual void BindDynamicTexture(u32 binding, const std::shared_ptr<DynamicTextureBuffer>& dTextureBuffer) = 0;

		static std::unique_ptr<BaseShader> CreateShader(const std::span<ShaderData>& shaderDatas);
	};
}
