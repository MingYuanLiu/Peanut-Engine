#pragma once
#include "runtime/core/framework/world/world.h"

namespace peanut{
    class World;
namespace reflection{
namespace TypeReflectionOparator{
    class WorldOperator{
    public:
        static const char* GetClassName(){ return "World";}
        static void* ConstructFromJson(const Json& json_context){
            World* ret_instance= new World;
            Serializer::read(json_context, *ret_instance);
            return ret_instance;
        }
        static Json WriteByName(void* instance){
            return Serializer::write(*(World*)instance);
        }
        // base class
        static int GetWorldBaseClassReflectionInstanceList(ReflectionInstance* &out_list, void* instance){
            int count = 0;
            
            return count;
        }
        // fields
        static const char* GetFieldName_world_name_(){ return "world_name_";}
        static const char* GetFieldTypeName_world_name_(){ return "std::string";}
        static void Set_world_name_(void* instance, void* field_value){ static_cast<World*>(instance)->world_name_ = *static_cast<std::string*>(field_value);}
        static void* Get_world_name_(void* instance){ return static_cast<void*>(&(static_cast<World*>(instance)->world_name_));}
        static bool IsArray_world_name_(){ return false; }
        static const char* GetFieldName_current_level_(){ return "current_level_";}
        static const char* GetFieldTypeName_current_level_(){ return "Level*";}
        static void Set_current_level_(void* instance, void* field_value){ static_cast<World*>(instance)->current_level_ = *static_cast<Level**>(field_value);}
        static void* Get_current_level_(void* instance){ return static_cast<void*>(&(static_cast<World*>(instance)->current_level_));}
        static bool IsArray_current_level_(){ return false; }

        // methods
        static const char* GetMethodName_GetWorldName(){ return "GetWorldName";}
        static void Invoke_GetWorldName(void * instance){static_cast<World*>(instance)->GetWorldName();}
        static const char* GetMethodName_GetCurrentLevel(){ return "GetCurrentLevel";}
        static void Invoke_GetCurrentLevel(void * instance){static_cast<World*>(instance)->GetCurrentLevel();}
    };
}//namespace TypeReflectionOparator


    void TypeWrapperRegister_World(){
		FieldFunctions* field_function_tuple_world_name_=new FieldFunctions(
            &TypeReflectionOparator::WorldOperator::Set_world_name_,
            &TypeReflectionOparator::WorldOperator::Get_world_name_,
            &TypeReflectionOparator::WorldOperator::GetClassName,
            &TypeReflectionOparator::WorldOperator::GetFieldName_world_name_,
            &TypeReflectionOparator::WorldOperator::GetFieldTypeName_world_name_,
            &TypeReflectionOparator::WorldOperator::IsArray_world_name_);
        REGISTER_FIELD_TO_MAP("World", field_function_tuple_world_name_);
		FieldFunctions* field_function_tuple_current_level_=new FieldFunctions(
            &TypeReflectionOparator::WorldOperator::Set_current_level_,
            &TypeReflectionOparator::WorldOperator::Get_current_level_,
            &TypeReflectionOparator::WorldOperator::GetClassName,
            &TypeReflectionOparator::WorldOperator::GetFieldName_current_level_,
            &TypeReflectionOparator::WorldOperator::GetFieldTypeName_current_level_,
            &TypeReflectionOparator::WorldOperator::IsArray_current_level_);
        REGISTER_FIELD_TO_MAP("World", field_function_tuple_current_level_);

        MethodFunctions* method_function_tuple_GetWorldName=new MethodFunctions(
            &TypeFieldReflectionOparator::TypeWorldOperator::GetMethodName_GetWorldName,
            &TypeFieldReflectionOparator::TypeWorldOperator::Invoke_GetWorldName);
        REGISTER_Method_TO_MAP("World", method_function_tuple_GetWorldName);
        MethodFunctions* method_function_tuple_GetCurrentLevel=new MethodFunctions(
            &TypeFieldReflectionOparator::TypeWorldOperator::GetMethodName_GetCurrentLevel,
            &TypeFieldReflectionOparator::TypeWorldOperator::Invoke_GetCurrentLevel);
        REGISTER_Method_TO_MAP("World", method_function_tuple_GetCurrentLevel);
        
        
		
		ClassFunctions* class_function_tuple_World=new ClassFunctions(
            &TypeFieldReflectionOparator::TypeWorldOperator::ConstructorWithJson,
            &TypeFieldReflectionOparator::TypeWorldOperator::WriteByName,
			&TypeFieldReflectionOparator::TypeWorldOperator::GetWorldBaseClassReflectionInstanceList);
        REGISTER_BASE_CLASS_TO_MAP("World", class_function_tuple_World);
    }
namespace TypeWrappersRegister{
    void World()
    {
        TypeWrapperRegister_World();
    }
}//namespace TypeWrappersRegister

}//namespace reflection
}//namespace peanut

