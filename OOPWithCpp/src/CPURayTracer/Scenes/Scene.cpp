#include "Core.hpp"
#include "Scene.hpp"
#include "BasicScene.hpp"
#include "RandTestScene.hpp"
#include "DuelWhiteSpheres.hpp"
#include "DielectricTest.hpp"
#include "MetalTest.hpp"
#include "EarthScene.hpp"
#include "Book1FinalRender.hpp"


namespace OWC
{
	std::unique_ptr<BaseScene> BaseScene::CreateScene(Scene scene)
	{
		switch (scene)
		{
		case Scene::Basic:
			return std::make_unique<BasicScene>();
//		case Scene::RandTest:
//			return std::make_unique<RandTestScene>();
		case Scene::DuelGraySpheres:
			return std::make_unique<DuelGraySpheres>();
		case Scene::DielectricTest:
			return std::make_unique<DielectricTest>();
		case Scene::MetalTest:
			return std::make_unique<MetalTest>();
		case Scene::EarthScene:
			return std::make_unique<EarthScene>();
		case Scene::Book1FinalRender:
			return std::make_unique<Book1FinalRender>();
		default:
			// Return Basic scene as default
			return std::make_unique<BasicScene>();
		}
	}
}