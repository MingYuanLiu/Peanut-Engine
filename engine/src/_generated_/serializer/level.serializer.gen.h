#pragma once
#include "runtime/core/serialize/serializer.h"
#include "runtime/core/framework/level/level.h"

namespace Piccolo{
    template<>
    Json Serializer::Write(const Level& instance) {
		Json::object ret_context;
        
		ret_context.insert_or_assign("level_name_", Serializer::Write(instance.level_name_));
		
		return Json(ret_context);
	}
    template<>
    Level& Serializer::Read(const Json& json_context, Level& instance) {
		assert(json_context.is_object());
		
        if(!json_context["level_name_"].is_null()){
            Serializer::Read(json_context["level_name_"], instance.level_name_);
        }
		return instance;
	}
}//namespace

