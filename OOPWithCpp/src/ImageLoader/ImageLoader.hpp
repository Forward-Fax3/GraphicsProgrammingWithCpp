#pragma once
#include "Core.hpp"
#include <string_view>
#include <vector>

#include <glm/glm.hpp>


namespace OWC
{
	class ImageLoader
	{
	public:
		ImageLoader() = delete;
		ImageLoader(const ImageLoader&) = delete;
		ImageLoader& operator=(const ImageLoader&) = delete;
		ImageLoader(ImageLoader&&) = delete;
		ImageLoader& operator=(ImageLoader&&) = delete;
		ImageLoader(std::string_view path);
		virtual ~ImageLoader() = default;

		[[nodiscard]] const std::vector<Vec4>& GetImageData() const { return m_ImageData; }
		[[nodiscard]] const Vec4& GetPixel(uSize x, uSize y) const { return m_ImageData[y * m_Width + x]; }
		// expects x and y to be in the range [0.0, 1.0]
		[[nodiscard]] const Vec4& GetPixel(f32 x, f32 y) const { return m_ImageData[static_cast<uSize>(y * static_cast<f32>(m_Height)) * m_Width + static_cast<uSize>(x * static_cast<f32>(m_Width))]; }
		[[nodiscard]] uSize GetWidth() const { return m_Width; }
		[[nodiscard]] uSize GetHeight() const { return m_Height; }

	private:
		std::vector<Vec4> m_ImageData;
		uSize m_Width = 0;
		uSize m_Height = 0;
	};
}
 