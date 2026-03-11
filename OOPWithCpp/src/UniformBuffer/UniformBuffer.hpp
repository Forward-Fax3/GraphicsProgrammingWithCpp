#pragma once
#include "ImageLoader.hpp"

#include <cstddef>
#include <span>
#include <memory>
#include <vector>
#include <glm/vec4.hpp>


namespace OWC::Graphics
{
	class UniformBuffer
	{
	public:
		UniformBuffer() = default;
		virtual ~UniformBuffer() = default;
		UniformBuffer(UniformBuffer&) = default;
		UniformBuffer& operator=(UniformBuffer&) = default;
		UniformBuffer(UniformBuffer&&) noexcept = default;
		UniformBuffer& operator=(UniformBuffer&&) noexcept = default;

		virtual void UpdateBufferData(std::span<const std::byte> data) = 0;

		static std::shared_ptr<UniformBuffer> CreateUniformBuffer(uSize size);
	};

	class TextureBuffer
	{
	public:
		TextureBuffer() = default;
		virtual ~TextureBuffer() = default;
		TextureBuffer(TextureBuffer&) = default;
		TextureBuffer& operator=(TextureBuffer&) = default;
		TextureBuffer(TextureBuffer&&) noexcept = default;
		TextureBuffer& operator=(TextureBuffer&&) noexcept = default;

		virtual void UpdateBufferData(const std::vector<Vec4>& data) = 0;

		static std::shared_ptr<TextureBuffer> CreateTextureBuffer(const ImageLoader& image);
		static std::shared_ptr<TextureBuffer> CreateTextureBuffer(u32 width, u32 height);
	};

	class DynamicTextureBuffer // a texture that can be updated every frame
	{
		public:
		DynamicTextureBuffer() = default;
		virtual ~DynamicTextureBuffer() = default;
		DynamicTextureBuffer(DynamicTextureBuffer&) = default;
		DynamicTextureBuffer& operator=(DynamicTextureBuffer&) = default;
		DynamicTextureBuffer(DynamicTextureBuffer&&) noexcept = default;
		DynamicTextureBuffer& operator=(DynamicTextureBuffer&&) noexcept = default;
		virtual void UpdateBufferData(const std::vector<Vec4>& data) = 0;

		static std::shared_ptr<DynamicTextureBuffer> CreateDynamicTextureBuffer(u32 width, u32 height);
	};
}
