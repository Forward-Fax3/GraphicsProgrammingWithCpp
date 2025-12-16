#include "Scene.hpp"
#include "BasicScene.hpp"


namespace OWC
{
	std::unique_ptr<BaseScene> BaseScene::CreateScene(Scene scene, std::vector<glm::vec4>& frameBuffer)
	{
		switch (scene)
		{
		case Scene::Basic:
			return std::make_unique<BasicScene>(frameBuffer);
		default:
			// Return Basic scene as default
			return std::make_unique<BasicScene>(frameBuffer);
		}
	}
}