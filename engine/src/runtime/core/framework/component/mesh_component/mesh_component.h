#pragma once

#include "runtime/core/reflection/marcos.h"
#include "runtime/core/framework/component/component.h"

namespace peanut
{
	class META_CLASS() StaticMeshComponent : public Component
	{
		GENERATE_BODY(StaticMeshComponent)
	public:

		StaticMeshComponent() = default;
		virtual ~StaticMeshComponent() = default;

		META_FUNCTION()
		virtual void Tick(float delta_time) override {}

		// void SetStaticMesh(std::shared_ptr<StaticMesh> static_mesh) { static_mesh_ = static_mesh; }

		// std::shared_ptr<StaticMesh> GetStaticMesh() const { return static_mesh_; }

		// std::shared_ptr<Material> GetMaterial() const { return material_; }

	private:
		// std::shared_ptr<StaticMesh> static_mesh_;

		// std::shared_ptr<Material> material_;

		// BoundingBox bounding_box_;

	};
} // namespace peanut