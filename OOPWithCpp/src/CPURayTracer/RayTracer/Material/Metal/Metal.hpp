#include <Core.hpp>

#include "BaseMaterial.hpp"

#include "BaseTexture.hpp"

#include <memory>


namespace OWC
{
	class Metal : public BaseMaterial
	{
	public:
		Metal() = delete;
		Metal(f32 roughness);
		Metal(f32 roughness, const Colour& colour);
		Metal(f32 roughness, const std::shared_ptr<BaseTexture>& texture);
		~Metal() override = default;

		Metal(const Metal&) = delete;
		Metal& operator=(const Metal&) = delete;
		Metal(Metal&&) = delete;
		Metal& operator=(Metal&&) = delete;

		bool Scatter(Ray& ray, const HitData& hitData) const override;
		Colour Albedo(HitData& data) const override;

	private:
		std::shared_ptr<BaseTexture> m_Texture = nullptr;
		f32 m_Roughness = 0.5f;
	};
}