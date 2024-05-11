#pragma once

#include "runtime/core/reflection/marcos.h"
#include "_generated_/reflection/all_reflection.h"

namespace peanut
{
	class META_CLASS() GameObject
	{
		GENERATE_BODY(GameObject);

	public:
		
		META_FUNCTION()
		std::string GetName() { return name_; }

	private:
		META_PROPERTY()
		std::string name_;

		META_PROPERTY()
		Level* parent_level_;
	};
}