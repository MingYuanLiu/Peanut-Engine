#pragma once
#include "runtime/core/reflection/reflection_core.h"
#include "_generated_/serializer/all_serializer.h"
#include "_generated_/reflection/level.reflection.gen.h"
#include "_generated_/reflection/world.reflection.gen.h"

namespace peanut{
namespace reflection{
    void TypeMetaRegister::MetaRegister(){
        TypeWrappersRegister::Level();
        TypeWrappersRegister::World();
    }
}
}

