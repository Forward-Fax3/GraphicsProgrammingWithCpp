#pragma once
#include <memory>
#include <ImageLoader.hpp>


namespace OWC
{
	class Image
	{
	public:
		Image() = default;
		virtual ~Image() = default;
		Image(const Image&) = delete;
		Image& operator=(const Image&) = delete;
		Image(Image&&) = delete;
		Image& operator=(Image&&) = delete;

		static std::shared_ptr<Image> LoadFromFile(const ImageLoader& image);
	};
}
