#pragma once

#include "runtime/core/reflection/marcos.h"
#include "runtime/core/framework/component/component.h"

namespace peanut
{
	class META_CLASS() MeshComponent : public Component
	{
		GENERATE_BODY(MeshComponent)
	public:

		MeshComponent() = default;
		virtual ~MeshComponent() = default;

		META_FUNCTION()
		virtual void Tick(float delta_time) override {}

		// void LoadStaticMesh(const std::string& path);

		// void SetMaterial(const Material& material) { material_ = material; }

		// void SetStaticMesh(std::shared_ptr<StaticMesh> static_mesh) { static_mesh_ = static_mesh; }

	private:
		// std::shared_ptr<StaticMesh> static_mesh_;

		// Material material_;

	};
} // namespace peanut