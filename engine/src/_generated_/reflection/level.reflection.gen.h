#pragma once
#include "runtime/core/reflection/reflection_core.h"
#include "runtime/core/framework/level/level.h"

namespace peanut{
    class Level;
namespace reflection{
namespace TypeReflectionOparator{
    class LevelOperators{
    public:
        static const char* GetClassName(){ return "Level";}
        static void* ConstructFromJson(const Json& json_context){
            Level* ret_instance= new Level;
            Serializer::Read(json_context, *ret_instance);
            return ret_instance;
        }
        static Json WriteByName(void* instance){
            return Serializer::Write(*(Level*)instance);
        }
        // base class
        static int GetLevelBaseClassReflectionInstanceList(ReflectionInstance* &out_list, void* instance){
            int count = 0;
            
            return count;
        }
        // fields
        static const char* GetFieldName_level_name_(){ return "level_name_";}
        static const char* GetFieldTypeName_level_name_(){ return "std::string";}
        static void Set_level_name_(void* instance, void* field_value){ static_cast<Level*>(instance)->level_name_ = *static_cast<std::string*>(field_value);}
        static void* Get_level_name_(void* instance){ return static_cast<void*>(&(static_cast<Level*>(instance)->level_name_));}
        static bool IsArray_level_name_(){ return false; }

        // methods
        static const char* GetMethodName_GetLevelName(){ return "GetLevelName";}
        static void Invoke_GetLevelName(void * instance){static_cast<Level*>(instance)->GetLevelName();}
    };
}//namespace TypeReflectionOparator


    void TypeWrapperRegister_Level(){
		FieldFunctions* field_function_tuple_level_name_=new FieldFunctions(
            &TypeReflectionOparator::LevelOperators::Set_level_name_,
            &TypeReflectionOparator::LevelOperators::Get_level_name_,
            &TypeReflectionOparator::LevelOperators::GetClassName,
            &TypeReflectionOparator::LevelOperators::GetFieldName_level_name_,
            &TypeReflectionOparator::LevelOperators::GetFieldTypeName_level_name_,
            &TypeReflectionOparator::LevelOperators::IsArray_level_name_);
        REGISTER_FIELD_TO_MAP("Level", field_function_tuple_level_name_);

        MethodFunctions* method_function_tuple_GetLevelName=new MethodFunctions(
            &TypeReflectionOparator::LevelOperators::GetMethodName_GetLevelName,
            &TypeReflectionOparator::LevelOperators::Invoke_GetLevelName);
        REGISTER_METHOD_TO_MAP("Level", method_function_tuple_GetLevelName);
        
        
		
		ClassFunctions* class_function_tuple_Level=new ClassFunctions(
            &TypeReflectionOparator::LevelOperators::ConstructFromJson,
            &TypeReflectionOparator::LevelOperators::WriteByName,
			&TypeReflectionOparator::LevelOperators::GetLevelBaseClassReflectionInstanceList);
        REGISTER_BASE_CLASS_TO_MAP("Level", class_function_tuple_Level);
    }
namespace TypeWrappersRegister{
    void Level()
    {
        TypeWrapperRegister_Level();
    }
}//namespace TypeWrappersRegister

}//namespace reflection
}//namespace peanut

