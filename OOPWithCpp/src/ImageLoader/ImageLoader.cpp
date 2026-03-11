#define STB_IMAGE_IMPLEMENTATION
#include "Core.hpp"
#include "ImageLoader.hpp"
#include "Log.hpp"

#include <stb_image.h>
#include <memory>


namespace OWC
{
	ImageLoader::ImageLoader(std::string_view path)
	{
		i32 tempWidth = 0;
		i32 tempHeight = 0;
		std::unique_ptr<f32[], decltype([](f32 ptr[]) { STBI_FREE(ptr); })> imageDataPtr(stbi_loadf(path.data(), &tempWidth, &tempHeight, nullptr, 4));

		if (!imageDataPtr)
		{
			std::string pathCopy(path);

			for (char& c : pathCopy)
				if (c == '\\')
					c = '/';

			imageDataPtr.reset(stbi_loadf(pathCopy.data(), &tempWidth, &tempHeight, nullptr, 4));

			if (!imageDataPtr)
				for (uSize i = 0; i < 16; i++)
				{
					pathCopy = "../" + pathCopy;
					imageDataPtr.reset(stbi_loadf(pathCopy.data(), &tempWidth, &tempHeight, nullptr, 4));
					if (imageDataPtr)
						break;
				}

			if (!imageDataPtr)
				Log<LogLevel::Error>("Failed to load image from path: {}", path);
		}

		m_Width = static_cast<uSize>(tempWidth);
		m_Height = static_cast<uSize>(tempHeight);

		m_ImageData.reserve(m_Width * m_Height);

		for (uSize i = 0; i < m_Height * m_Width * 4; i += 4)
			m_ImageData.emplace_back(
				imageDataPtr[i + 0],	// R
				imageDataPtr[i + 1],	// G
				imageDataPtr[i + 2],	// B
				imageDataPtr[i + 3]		// A
			);
	}
}