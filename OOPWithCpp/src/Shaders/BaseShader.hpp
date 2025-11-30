#pragma once
#include <string>
#include <vector>
#include <memory>


namespace OWC::Graphics
{
	struct ShaderData
	{
		enum class ShaderType : uint8_t
		{
			Vertex,
			Fragment,
			Compute
		};

		enum class ShaderLanguage : uint8_t
		{ // for now only SPIR-V is supported
			GLSL,
			HLSL,
			Slang,
			SPIRV,

			SPV = SPIRV
		};

		std::string ShaderTypeToString() const
		{
			switch (type)
			{
			using enum OWC::Graphics::ShaderData::ShaderType;
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
		std::vector<uint32_t> bytecode; // for SPIR-V
		
		ShaderType type;
		ShaderLanguage language;
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

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		static std::unique_ptr<BaseShader> CreateShader(const std::vector<ShaderData>& shaderDatas);
	};
}
