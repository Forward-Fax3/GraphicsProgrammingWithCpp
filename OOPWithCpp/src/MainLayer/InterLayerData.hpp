#pragma once
#include "Core.hpp"
#include <vector>
#include <cstdint>
#include <bitset>
#include <glm/vec4.hpp>


namespace OWC
{
	struct InterLayerData
	{
		std::vector<Vec4> imageData{};
		uSize numberOfSamples = 0;
		u32 imageWidth = 0;
		u32 imageHeight = 0;
		std::bitset<2> ImageUpdates; // bit 0: update image, bit 1: image resize
		f32 invGammaValue = 1.0f / 2.2f;
	};
}
