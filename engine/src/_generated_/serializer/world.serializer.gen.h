#pragma once
#include "runtime/core/framework/world/world.h"

namespace Piccolo{
    template<>
    Json Serializer::write(const World& instance);
    template<>
    World& Serializer::read(const Json& json_context, World& instance);
}//namespace

