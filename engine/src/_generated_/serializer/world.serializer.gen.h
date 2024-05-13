#pragma once
#include "runtime/core/serialize/serializer.h"
#include "runtime/core/framework/world/world.h"

namespace Piccolo{
    template<>
    Json Serializer::Write(const World& instance) {
		Json::object ret_context;
        
		ret_context.insert_or_assign("world_name_", Serializer::Write(instance.world_name_));
		ret_context.insert_or_assign("current_level_", Serializer::Write(instance.current_level_));
        Json::array levels__json;
		for (auto& item : instance.levels_){
            levels__json.emplace_back(Serializer::Write(item));
        }
		ret_context.insert_or_assign("levels_",levels__json);
		
		
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
        if(!json_context["levels_"].is_null()){
            assert(json_context["levels_"].is_array());
            Json::array array_levels_ = json_context["levels_"].array_items();
            instance.levels_.resize(array_levels_.size());
            for (size_t index=0; index < array_levels_.size();++index){
                Serializer::Read(array_levels_[index], instance.levels_[index]);
            }
        }
		return instance;
	}
}//namespace

