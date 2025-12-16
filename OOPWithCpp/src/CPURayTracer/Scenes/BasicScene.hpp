#pragma once
#include "Scene.hpp"
#include "BaseHittable.hpp"


namespace OWC
{
	class BasicScene : public BaseScene
	{
	public:
		explicit BasicScene() = delete;
		BasicScene(std::vector<Vec4>& frameBuffer);
		~BasicScene() override = default;

		BasicScene(const BasicScene&) = delete;
		BasicScene& operator=(const BasicScene&) = delete;
		BasicScene(BasicScene&&) = delete;
		BasicScene& operator=(BasicScene&&) = delete;

		bool RenderNextPass() override;

	private:
		std::unique_ptr<BaseHittable> m_Hittables;
	};
}
