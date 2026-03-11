#include "Image.hpp"
#include "VulkanImage.hpp"


namespace OWC
{
	std::shared_ptr<Image> Image::LoadFromFile(const ImageLoader& image)
	{
//		return std::make_shared<VulkanImage>(image);
		(void)image;
		return nullptr;
	}
}
