#pragma once

#include "runtime/core/reflection/marcos.h"
#include "runtime/core/framework/component/tickable.h"
#include "_generated_/reflection/all_reflection.h"

namespace peanut
{
	class GameObject;
	class META_CLASS() Component : public ITickable
	{
		GENERATE_BODY(Component)
	public:
		Component() = default;
		virtual ~Component() = default;

		META_FUNCTION()
		virtual void Attach(const std::weak_ptr<GameObject>&parent) {}

		META_FUNCTION()
		virtual void Detach() {}

		META_FUNCTION()
		virtual void BeginPlay() {}

		META_FUNCTION()
		virtual void EndPlay() {}

		META_FUNCTION()
		virtual void Tick(float delta_time) override {}

		std::weak_ptr<GameObject> GetParent() const { return parent_; }

		bool IsDirty() const { return is_dirty_; }
		void SetDirty(bool dirty) { is_dirty_ = dirty; }
	protected:
		META_PROPERTY()
		std::weak_ptr<GameObject> parent_;

		META_PROPERTY()
		bool is_dirty_{ false };
	};
}