#pragma once
#include "world.serializer.gen.h"
#include "component.serializer.gen.h"
#include "game_object.serializer.gen.h"
#include "level.serializer.gen.h"
namespace peanut{
    template<>
    Json Serializer::write(const World& instance){
        Json::object  ret_context;
        
        ret_context.insert_or_assign("world_name_", Serializer::write(instance.world_name_));
        ret_context.insert_or_assign("current_level_", Serializer::write(instance.current_level_));
        Json::array levels__json;
        for (auto& item : instance.levels_){
            levels__json.emplace_back(Serializer::write(item));
        }
        ret_context.insert_or_assign("levels_",levels__json);
        
        return  Json(ret_context);
    }
    template<>
    World& Serializer::read(const Json& json_context, World& instance){
        assert(json_context.is_object());
        
        if(!json_context["world_name_"].is_null()){
            Serializer::read(json_context["world_name_"], instance.world_name_);
        }
        if(!json_context["current_level_"].is_null()){
            Serializer::read(json_context["current_level_"], instance.current_level_);
        }
        if(!json_context["levels_"].is_null()){
            assert(json_context["levels_"].is_array());
            Json::array array_levels_ = json_context["levels_"].array_items();
            instance.levels_.resize(array_levels_.size());
            for (size_t index=0; index < array_levels_.size();++index){
                Serializer::read(array_levels_[index], instance.levels_[index]);
            }
        }
        return instance;
    }
    template<>
    Json Serializer::write(const Component& instance){
        Json::object  ret_context;
        
        ret_context.insert_or_assign("parent_", Serializer::write(instance.parent_));
        ret_context.insert_or_assign("is_dirty_", Serializer::write(instance.is_dirty_));
        return  Json(ret_context);
    }
    template<>
    Component& Serializer::read(const Json& json_context, Component& instance){
        assert(json_context.is_object());
        
        if(!json_context["parent_"].is_null()){
            Serializer::read(json_context["parent_"], instance.parent_);
        }
        if(!json_context["is_dirty_"].is_null()){
            Serializer::read(json_context["is_dirty_"], instance.is_dirty_);
        }
        return instance;
    }
    template<>
    Json Serializer::write(const GameObject& instance){
        Json::object  ret_context;
        
        ret_context.insert_or_assign("name_", Serializer::write(instance.name_));
        ret_context.insert_or_assign("guid_", Serializer::write(instance.guid_));
        ret_context.insert_or_assign("parent_level_", Serializer::write(instance.parent_level_));
        Json::array components__json;
        for (auto& item : instance.components_){
            components__json.emplace_back(Serializer::write(item));
        }
        ret_context.insert_or_assign("components_",components__json);
        
        return  Json(ret_context);
    }
    template<>
    GameObject& Serializer::read(const Json& json_context, GameObject& instance){
        assert(json_context.is_object());
        
        if(!json_context["name_"].is_null()){
            Serializer::read(json_context["name_"], instance.name_);
        }
        if(!json_context["guid_"].is_null()){
            Serializer::read(json_context["guid_"], instance.guid_);
        }
        if(!json_context["parent_level_"].is_null()){
            Serializer::read(json_context["parent_level_"], instance.parent_level_);
        }
        if(!json_context["components_"].is_null()){
            assert(json_context["components_"].is_array());
            Json::array array_components_ = json_context["components_"].array_items();
            instance.components_.resize(array_components_.size());
            for (size_t index=0; index < array_components_.size();++index){
                Serializer::read(array_components_[index], instance.components_[index]);
            }
        }
        return instance;
    }
    template<>
    Json Serializer::write(const Level& instance){
        Json::object  ret_context;
        
        ret_context.insert_or_assign("level_name_", Serializer::write(instance.level_name_));
        Json::array game_objects__json;
        for (auto& item : instance.game_objects_){
            game_objects__json.emplace_back(Serializer::write(item));
        }
        ret_context.insert_or_assign("game_objects_",game_objects__json);
        
        return  Json(ret_context);
    }
    template<>
    Level& Serializer::read(const Json& json_context, Level& instance){
        assert(json_context.is_object());
        
        if(!json_context["level_name_"].is_null()){
            Serializer::read(json_context["level_name_"], instance.level_name_);
        }
        if(!json_context["game_objects_"].is_null()){
            assert(json_context["game_objects_"].is_array());
            Json::array array_game_objects_ = json_context["game_objects_"].array_items();
            instance.game_objects_.resize(array_game_objects_.size());
            for (size_t index=0; index < array_game_objects_.size();++index){
                Serializer::read(array_game_objects_[index], instance.game_objects_[index]);
            }
        }
        return instance;
    }

}

