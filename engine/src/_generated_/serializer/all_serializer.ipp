#pragma once
#include "level.serializer.gen.h"
#include "world.serializer.gen.h"
namespace peanut{
    template<>
    Json Serializer::write(const Level& instance){
        Json::object  ret_context;
        
        ret_context.insert_or_assign("level_name_", Serializer::write(instance.level_name_));
        return  Json(ret_context);
    }
    template<>
    Level& Serializer::read(const Json& json_context, Level& instance){
        assert(json_context.is_object());
        
        if(!json_context["level_name_"].is_null()){
            Serializer::read(json_context["level_name_"], instance.level_name_);
        }
        return instance;
    }
    template<>
    Json Serializer::write(const World& instance){
        Json::object  ret_context;
        
        ret_context.insert_or_assign("world_name_", Serializer::write(instance.world_name_));
        ret_context.insert_or_assign("current_level_", Serializer::write(instance.current_level_));
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
        return instance;
    }

}

