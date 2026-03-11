#pragma once
#include "Core.hpp"
#include "BaseTexture.hpp"

#include "ImageLoader.hpp"


namespace OWC
{
    class ImageTexture : public BaseTexture
    {
    public:
        ImageTexture(const std::string& imagePath);
		~ImageTexture() override = default;

		ImageTexture(const ImageTexture&) = delete;
		ImageTexture& operator=(const ImageTexture&) = delete;
		ImageTexture(ImageTexture&&) = delete;
		ImageTexture& operator=(ImageTexture&&) = delete;

        Colour Value(const HitData& p) const override;

	private:
		ImageLoader m_Image;
    };
}
