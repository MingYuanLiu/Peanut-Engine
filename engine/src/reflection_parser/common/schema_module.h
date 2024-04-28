#pragma once
#include "precompiled.h"

class MetaClass;
class Global;
class Function;
class Enum;

// use SchemaModule to record multiple classes in a file
struct SchemaMoudle
{
    std::string name; // class name

    std::vector<std::shared_ptr<MetaClass>> classes;
};