#pragma once
#include "runtime/core/serialize/serializer.h"
#include "runtime/core/framework/level/level.h"

namespace Piccolo{
    template<>
    Json Serializer::Write(const Level& instance) {
		Json::object ret_context;
        
		ret_context.insert_or_assign("level_name_", Serializer::Write(instance.level_name_));
        Json::array game_objects__json;
		for (auto& item : instance.game_objects_){
            game_objects__json.emplace_back(Serializer::Write(item));
        }
		ret_context.insert_or_assign("game_objects_",game_objects__json);
		
		
		return Json(ret_context);
	}
    template<>
    Level& Serializer::Read(const Json& json_context, Level& instance) {
		assert(json_context.is_object());
		
        if(!json_context["level_name_"].is_null()){
            Serializer::Read(json_context["level_name_"], instance.level_name_);
        }
        if(!json_context["game_objects_"].is_null()){
            assert(json_context["game_objects_"].is_array());
            Json::array array_game_objects_ = json_context["game_objects_"].array_items();
            instance.game_objects_.resize(array_game_objects_.size());
            for (size_t index=0; index < array_game_objects_.size();++index){
                Serializer::Read(array_game_objects_[index], instance.game_objects_[index]);
            }
        }
		return instance;
	}
}//namespace

