#pragma once
#include <vector>
#include <cstdint>
#include <bitset>
#include <glm/vec4.hpp>


namespace OWC
{
	struct InterLayerData
	{
		std::vector<glm::vec4> imageData{};
		size_t numberOfSamples = 0;
		uint32_t imageWidth = 0;
		uint32_t imageHeight = 0;
		std::bitset<2> ImageUpdates; // bit 0: update image, bit 1: image resize
	};
}