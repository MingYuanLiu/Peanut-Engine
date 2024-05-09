#pragma once
#include "runtime/core/framework/level/level.h"

namespace Piccolo{
    template<>
    Json Serializer::write(const Level& instance);
    template<>
    Level& Serializer::read(const Json& json_context, Level& instance);
}//namespace

