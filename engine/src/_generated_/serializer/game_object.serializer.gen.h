#pragma once
#include "runtime/core/serialize/serializer.h"
#include "runtime/core/framework/game_object/game_object.h"

namespace peanut{
    template<>
    Json Serializer::Write(const GameObject& instance) {
		Json::object ret_context;
        
		ret_context.insert_or_assign("name_", Serializer::Write(instance.name_));
		ret_context.insert_or_assign("guid_", Serializer::Write(instance.guid_));
		ret_context.insert_or_assign("parent_level_", Serializer::Write(instance.parent_level_));
        Json::array components__json;
		for (auto& item : instance.components_){
            components__json.emplace_back(Serializer::Write(item));
        }
		ret_context.insert_or_assign("components_",components__json);
		
		
		return Json(ret_context);
	}
    template<>
    GameObject& Serializer::Read(const Json& json_context, GameObject& instance) {
		assert(json_context.is_object());
		
        if(!json_context["name_"].is_null()){
            Serializer::Read(json_context["name_"], instance.name_);
        }
        if(!json_context["guid_"].is_null()){
            Serializer::Read(json_context["guid_"], instance.guid_);
        }
        if(!json_context["parent_level_"].is_null()){
            Serializer::Read(json_context["parent_level_"], instance.parent_level_);
        }
        if(!json_context["components_"].is_null()){
            assert(json_context["components_"].is_array());
            Json::array array_components_ = json_context["components_"].array_items();
            instance.components_.resize(array_components_.size());
            for (size_t index=0; index < array_components_.size();++index){
                Serializer::Read(array_components_[index], instance.components_[index]);
            }
        }
		return instance;
	}
}//namespace

