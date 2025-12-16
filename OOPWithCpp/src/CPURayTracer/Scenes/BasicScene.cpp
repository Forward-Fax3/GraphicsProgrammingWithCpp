#include "Application.hpp"
#include "BasicScene.hpp"
#include "Sphere.hpp"
#include "Ray.hpp"
#include "OWCRand.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/vec_swizzle.hpp>


namespace OWC
{
	BasicScene::BasicScene(std::vector<glm::vec4>& frameBuffer)
		: BaseScene(frameBuffer)
	{
		m_Hittables = std::make_unique<Sphere>(glm::vec3(0.0f, 0.0f, -5.0f), 1.0f);
	}

	bool BasicScene::RenderNextPass()
	{
		const auto& app = Application::GetConstInstance();
		auto windowWidth = app.GetWindowWidth();
		auto windowHeight = app.GetWindowHeight();
		auto halfWidth = static_cast<float>(windowWidth) * 0.5f;
		auto halfHeight = static_cast<float>(windowHeight) * 0.5f;
		Vec2 halfScreenSize{ halfWidth, halfHeight };
		Vec2 invHalfScreenSize = 1.0f / halfScreenSize;
		Vec4 halfScreenSize_128 = glm::xyxy(halfScreenSize);
		Vec4 invHalfScreenSize_128 = glm::xyxy(invHalfScreenSize);

		std::vector<Vec4>& frameBuffer = GetFrameBuffer();
		size_t i = 0;

		for (; i < frameBuffer.size(); i += 2)
		{
			Vec4 pixelCoord {
				static_cast<float>(i % windowWidth),
				static_cast<float>(i / windowWidth),
				static_cast<float>((i + 1) % windowWidth),
				static_cast<float>((i + 1) / windowWidth)
			};
			pixelCoord += Rand::LinearFastRandVec4(Vec4(0.0f), Vec4(1.0f));
			pixelCoord -= halfScreenSize_128;
			pixelCoord *= invHalfScreenSize_128;
			// create Ray
			Ray ray {
				Vec3(0.0f),
				glm::normalize(Vec3(
					glm::xy(pixelCoord),
					-1.0f))
			};

			// check for hits with m_Hittables
			HitData hitData = m_Hittables->IsHit(ray);
			if (hitData.hasHit)
				frameBuffer[i] += Vec4(1.0f, 0.0f, 0.0f, 1.0f); // red color for hit
			else
				frameBuffer[i] += Vec4(0.0f, 0.0f, 0.0f, 1.0f); // black color for miss

			ray = Ray(
				Vec3(0.0f),
				glm::normalize(Vec3(
					glm::zw(pixelCoord),
					-1.0
				))
			);

			hitData = m_Hittables->IsHit(ray);
			if (hitData.hasHit)
				frameBuffer[i+1] += Vec4(1.0f, 0.0f, 0.0f, 1.0f); // red color for hit
			else
				frameBuffer[i+1] += Vec4(0.0f, 0.0f, 0.0f, 1.0f); // black color for miss
		}

		i -= 1;
		for (; i < frameBuffer.size(); i += 1)
		{
			Vec2 pixelCoord{
				static_cast<float>(i % windowWidth),
				static_cast<float>(i / windowWidth)
			};
			pixelCoord += Rand::LinearFastRandVec2(Vec2(0.0f), Vec2(1.0f));
			pixelCoord -= halfScreenSize;
			pixelCoord *= invHalfScreenSize;
			// create Ray
			Ray ray{
				Vec3(0.0f),
				glm::normalize(Vec3(
					pixelCoord,
					-1.0f))
			};

			// check for hits with m_Hittables
			HitData hitData = m_Hittables->IsHit(ray);
			if (hitData.hasHit)
				frameBuffer[i] += Vec4(1.0f, 0.0f, 0.0f, 1.0f); // red color for hit
			else
				frameBuffer[i] += Vec4(0.0f, 0.0f, 0.0f, 1.0f); // black color for miss
		}

		return true;
	}
}