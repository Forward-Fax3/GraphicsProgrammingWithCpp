#include "ImageTexture.hpp"


namespace OWC
{
	ImageTexture::ImageTexture(const std::string& imagePath)
		: m_Image(imagePath) {}

	OWC::Colour ImageTexture::Value(const HitData& hitData) const
	{
		Vec2 uv = glm::clamp(hitData.uv, Vec2(0.0f), Vec2(1.0f));
		uv.x = 1.0f - uv.x;

		auto x = static_cast<uSize>(uv.x * static_cast<f32>(m_Image.GetWidth() - 1));
		auto y = static_cast<uSize>(uv.y * static_cast<f32>(m_Image.GetHeight() - 1));
		return m_Image.GetPixel(x, y);
	}
}
