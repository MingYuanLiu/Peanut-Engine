#pragma once

#include "runtime/core/reflection/marcos.h"
#include "runtime/core/framework/component/component.h"
#include "runtime/core/framework/component/tickable.h"
#include "_generated_/reflection/all_reflection.h"

#include <string>
#include <memory>

namespace peanut
{
	class Level;

	using GO_Guid = uint64_t;

	class META_CLASS() GameObject : public std::enable_shared_from_this<GameObject>, public ITickable
	{
		GENERATE_BODY(GameObject);

	public:
		
		META_FUNCTION()
		std::string GetName() { return name_; }

		META_FUNCTION()
		GO_Guid GetGuid() { return guid_; }

		META_FUNCTION()
		void AddComponent(std::shared_ptr<Component> component) { components_.emplace_back(component); }

		META_FUNCTION()
		void RemoveComponent(std::shared_ptr<Component> component) {
			components_.erase(std::remove(components_.begin(), components_.end(), component), components_.end()); 
		}

		META_FUNCTION()
		virtual void Tick(float delta_time) override {}

	private:
		META_PROPERTY()
		std::string name_;

		META_PROPERTY()
		GO_Guid guid_;

		META_PROPERTY()
		std::weak_ptr<Level> parent_level_;

		META_PROPERTY()
		std::vector<std::shared_ptr<Component> > components_;
	};
}