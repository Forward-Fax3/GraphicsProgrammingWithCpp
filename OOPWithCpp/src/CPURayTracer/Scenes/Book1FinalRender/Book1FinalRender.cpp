#include "Book1FinalRender.hpp"

#include "Log.hpp"
#include "OWCRand.hpp"

#include "Sphere.hpp"
#include "SplitBVH.hpp"

#include "Lambertian.hpp"
#include "Dielectric.hpp"
#include "Metal.hpp"
#include "DefusedLight.hpp"

#include "SolidTexture.hpp"


namespace OWC
{
	Book1FinalRender::Book1FinalRender()
	{
		constexpr size_t sqrtNumRandomSpheres = 50;
		constexpr size_t numRandomSpheres = sqrtNumRandomSpheres * sqrtNumRandomSpheres;
		constexpr f32 halfsqrtNumRandomSpheresf32 = (static_cast<f32>(sqrtNumRandomSpheres) / 2.0f) + 0.5f;

		m_SceneObjects = std::make_shared<Hitables>();
		m_SceneObjects->Reserve(numRandomSpheres + 5);
		m_SceneObjects->SetBackgroundFunction([](const Ray& ray)
			{
				constexpr f32 scale = 0.4f;
				f32 t = 0.5f * (ray.GetDirection().y + 1.0f);
				return (1.0f - t) + t * Colour(0.5f, 0.7f, 1.0f, 1.0f) * scale;
			});

		std::shared_ptr<BaseMaterial> material = nullptr;

		// Light
		{
			material = std::make_shared<DefusedLight>(Colour(1.0f, 1.0f, 1.0f, 1.0f), 5.0f);
			m_SceneObjects->AddObject(std::make_shared<Sphere>(Point(500.0f, -500.0f, -500.0f), 44.72f, material));
		}
		// Glass Sphere
		{
			material = std::make_shared<Dielectric>(1.5f);
			m_SceneObjects->AddObject(std::make_shared<Sphere>(Point(0.0f, -1.0f, 0.0f), 1.0f, material));
		}
		// Lambertion Sphere
		{
			material = std::make_shared<Lambertian>(Colour(0.4f, 0.2f, 0.1f, 1.0f));
			m_SceneObjects->AddObject(std::make_shared<Sphere>(Point(-4.0f, -1, 0), 1.0, material));
		}
		// Metal Sphere
		{
			material = std::make_shared<Metal>(0.0f, Colour(0.7f, 0.6f, 0.5f, 1.0f));
			m_SceneObjects->AddObject(std::make_shared<Sphere>(Point(4.0f, -1.0f, 0.0f), 1.0f, material));
		}
		// Ground
		{
			material =  std::make_shared<Lambertian>(std::make_shared<SolidTexture>(Colour(0.4f, 0.5f, 0.4f, 1.0f)));
			m_SceneObjects->AddObject(std::make_shared<Sphere>(Point(0.0f, 1000.0f, 0.0f), 999.7f, material));
		}

		for (size_t i = 0; i < numRandomSpheres; i++)
		{
			Colour randColour = Rand::LinearFastRandVec4(Colour(0.0f, 0.0f, 0.0f, 1.0f), Colour(1.0f));
			Point randPoint(static_cast<f32>(i % sqrtNumRandomSpheres) - halfsqrtNumRandomSpheresf32, 0.1, static_cast<f32>(i / sqrtNumRandomSpheres) - halfsqrtNumRandomSpheresf32);
			randPoint += Rand::LinearFastRandVec3(Vec3(0.1f, 0.0f, 0.1f), Vec3(0.9f, 0.0f, 0.9f));

			switch (auto randInt = Rand::LinearFastRandValue(0, 3); randInt)
			{
			case 0: // Lambertion
			{
				material = std::make_shared<Lambertian>(randColour);
				break;
			}
			case 1: // Metal
			{
				material = std::make_shared<Metal>(Rand::LinearFastRandValue(0.0f, 1.0f), randColour);
				break;
			}
			case 2: // Dielectric
			{
				material = std::make_shared<Dielectric>(Rand::LinearFastRandValue(1.0f / 2.5f, 2.5f), randColour);
				break;
			}
			default:
			{
				material = std::make_shared<Lambertian>(Colour(Vec3(0.0f), 1.0f)); // black sphere avoids calling null material
				Log<LogLevel::Error>("Rand int gen gone very wrong expected: 0, 1, 2. Got {}\n created black sphere instead", randInt);
			}
			}

			m_SceneObjects->AddObject(std::make_shared<Sphere>(randPoint, 0.2f, material));
		}

		m_Hitable = std::make_shared<SplitBVH>(m_SceneObjects);
//		m_Hitable = m_SceneObjects;
	}

	void Book1FinalRender::SetBaseCameraSettings(CameraRenderSettings& cameraSettings) const
	{
		cameraSettings.Position = Point(8.5f, -2.0f, -8.5f);
		cameraSettings.Rotation = Vec3(-20.0f, -36.0f, 0.0f);
		cameraSettings.FOV = 50.0f;
		cameraSettings.FocalLength = 600.0f;
	}
}