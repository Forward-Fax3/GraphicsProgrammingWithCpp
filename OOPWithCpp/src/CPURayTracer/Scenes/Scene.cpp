#include "Core.hpp"
#include "Scene.hpp"
#include "BasicScene.hpp"
#include "DuelGreySpheres.hpp"
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
		case Scene::DuelGreySpheres:
			return std::make_unique<DuelGreySpheres>();
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