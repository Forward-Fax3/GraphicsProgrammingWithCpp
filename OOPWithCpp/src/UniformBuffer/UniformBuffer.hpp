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
		UniformBuffer& operator=(const UniformBuffer&) = default;
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
		TextureBuffer& operator=(const TextureBuffer&) = default;
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
		DynamicTextureBuffer& operator=(const DynamicTextureBuffer&) = default;
		DynamicTextureBuffer(DynamicTextureBuffer&&) noexcept = default;
		DynamicTextureBuffer& operator=(DynamicTextureBuffer&&) noexcept = default;
		virtual void UpdateBufferData(const std::vector<Vec4>& data) = 0;

		static std::shared_ptr<DynamicTextureBuffer> CreateDynamicTextureBuffer(u32 width, u32 height);
	};

	class GeneralBuffer
	{
	public:
		GeneralBuffer() = default;
		virtual ~GeneralBuffer() = default;
		GeneralBuffer(GeneralBuffer&) = default;
		GeneralBuffer& operator=(const GeneralBuffer&) = default;
		GeneralBuffer(GeneralBuffer&&) noexcept = default;
		GeneralBuffer& operator=(GeneralBuffer&&) noexcept = default;

		virtual void UpdateBufferDataImpl(const u8* data, uSize count, uSize offset) = 0;

		OWC_FORCE_INLINE void UpdateBufferData(const u8* data, const uSize count = 0, const uSize offset = 0)
		{
			UpdateBufferDataImpl(data, count, offset);
		}

		static std::shared_ptr<GeneralBuffer> CreateGeneralBuffer(uSize size);
	};

}
