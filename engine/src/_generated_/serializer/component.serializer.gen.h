#pragma once
#include "runtime/core/serialize/serializer.h"
#include "runtime/core/framework/component/component.h"

namespace peanut{
    template<>
    Json Serializer::Write(const Component& instance) {
		Json::object ret_context;
        
		ret_context.insert_or_assign("parent_", Serializer::Write(instance.parent_));
		ret_context.insert_or_assign("is_dirty_", Serializer::Write(instance.is_dirty_));
		
		return Json(ret_context);
	}
    template<>
    Component& Serializer::Read(const Json& json_context, Component& instance) {
		assert(json_context.is_object());
		
        if(!json_context["parent_"].is_null()){
            Serializer::Read(json_context["parent_"], instance.parent_);
        }
        if(!json_context["is_dirty_"].is_null()){
            Serializer::Read(json_context["is_dirty_"], instance.is_dirty_);
        }
		return instance;
	}
}//namespace

