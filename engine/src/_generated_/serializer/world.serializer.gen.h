#pragma once
#include "runtime/core/serialize/serializer.h"
#include "runtime/core/framework/world/world.h"

namespace Piccolo{
    template<>
    Json Serializer::Write(const World& instance) {
		Json::object ret_context;
        
		ret_context.insert_or_assign("world_name_", Serializer::Write(instance.world_name_));
		ret_context.insert_or_assign("current_level_", Serializer::Write(instance.current_level_));
		
		return Json(ret_context);
	}
    template<>
    World& Serializer::Read(const Json& json_context, World& instance) {
		assert(json_context.is_object());
		
        if(!json_context["world_name_"].is_null()){
            Serializer::Read(json_context["world_name_"], instance.world_name_);
        }
        if(!json_context["current_level_"].is_null()){
            Serializer::Read(json_context["current_level_"], instance.current_level_);
        }
		return instance;
	}
}//namespace

