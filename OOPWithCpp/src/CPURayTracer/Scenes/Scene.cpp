#include "Core.hpp"
#include "Scene.hpp"
#include "BasicScene.hpp"
#include "RandTestScene.hpp"
#include "DuelWhiteSpheres.hpp"


namespace OWC
{
	std::unique_ptr<BaseScene> BaseScene::CreateScene(Scene scene, std::vector<Vec4>& frameBuffer)
	{
		switch (scene)
		{
		case Scene::Basic:
			return std::make_unique<BasicScene>(frameBuffer);
		case Scene::RandTest:
			return std::make_unique<RandTestScene>(frameBuffer);
		case Scene::DuelGraySpheres:
			return std::make_unique<DuelGraySpheres>(frameBuffer);
		default:
			// Return Basic scene as default
			return std::make_unique<BasicScene>(frameBuffer);
		}
	}
}