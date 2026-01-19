#include "Core.hpp"
#include "Scene.hpp"
#include "BasicScene.hpp"
#include "RandTestScene.hpp"
#include "DuelWhiteSpheres.hpp"
#include "DielectricTest.hpp"


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
		case Scene::dielectricTest:
			return std::make_unique<DielectricTest>();
		default:
			// Return Basic scene as default
			return std::make_unique<BasicScene>();
		}
	}
}