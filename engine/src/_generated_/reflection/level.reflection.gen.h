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
        static const char* GetFieldName_game_objects_(){ return "game_objects_";}
        static const char* GetFieldTypeName_game_objects_(){ return "std::vector<GameObject*>";}
        static void Set_game_objects_(void* instance, void* field_value){ static_cast<Level*>(instance)->game_objects_ = *static_cast<std::vector<GameObject*>*>(field_value);}
        static void* Get_game_objects_(void* instance){ return static_cast<void*>(&(static_cast<Level*>(instance)->game_objects_));}
        static bool IsArray_game_objects_(){ return true; }

        // methods
        static const char* GetMethodName_GetLevelName(){ return "GetLevelName";}
        static void Invoke_GetLevelName(void * instance){static_cast<Level*>(instance)->GetLevelName();}
    };
}//namespace TypeReflectionOparator
namespace ArrayReflectionOperator{
#ifndef ArraystdSSvectorLGameObjectPROperatorMACRO
#define ArraystdSSvectorLGameObjectPROperatorMACRO
    class ArraystdSSvectorLGameObjectPROperators{
        public:
            static const char* GetArrayTypeName(){ return "std::vector<GameObject*>";}
            static const char* GetElementTypeName(){ return "GameObject*";}
            static int GetSize(void* instance){
                //todo: should check validation
                return static_cast<int>(static_cast<std::vector<GameObject*>*>(instance)->size());
            }
            static void* Get(int index,void* instance){
                //todo: should check validation
                return static_cast<void*>(&((*static_cast<std::vector<GameObject*>*>(instance))[index]));
            }
            static void Set(int index, void* instance, void* element_value){
                //todo: should check validation
                (*static_cast<std::vector<GameObject*>*>(instance))[index] = *static_cast<GameObject**>(element_value);
            }
    };
#endif //ArraystdSSvectorLGameObjectPROperator
}//namespace ArrayReflectionOperator

    void TypeWrapperRegister_Level(){
		FieldFunctions* field_function_tuple_level_name_=new FieldFunctions(
            &TypeReflectionOparator::LevelOperators::Set_level_name_,
            &TypeReflectionOparator::LevelOperators::Get_level_name_,
            &TypeReflectionOparator::LevelOperators::GetClassName,
            &TypeReflectionOparator::LevelOperators::GetFieldName_level_name_,
            &TypeReflectionOparator::LevelOperators::GetFieldTypeName_level_name_,
            &TypeReflectionOparator::LevelOperators::IsArray_level_name_);
        REGISTER_FIELD_TO_MAP("Level", field_function_tuple_level_name_);
		FieldFunctions* field_function_tuple_game_objects_=new FieldFunctions(
            &TypeReflectionOparator::LevelOperators::Set_game_objects_,
            &TypeReflectionOparator::LevelOperators::Get_game_objects_,
            &TypeReflectionOparator::LevelOperators::GetClassName,
            &TypeReflectionOparator::LevelOperators::GetFieldName_game_objects_,
            &TypeReflectionOparator::LevelOperators::GetFieldTypeName_game_objects_,
            &TypeReflectionOparator::LevelOperators::IsArray_game_objects_);
        REGISTER_FIELD_TO_MAP("Level", field_function_tuple_game_objects_);

        MethodFunctions* method_function_tuple_GetLevelName=new MethodFunctions(
            &TypeReflectionOparator::LevelOperators::GetMethodName_GetLevelName,
            &TypeReflectionOparator::LevelOperators::Invoke_GetLevelName);
        REGISTER_METHOD_TO_MAP("Level", method_function_tuple_GetLevelName);
        
		ArrayFunctions* array_tuple_stdSSvectorLGameObjectPR = new  ArrayFunctions(
            &ArrayReflectionOperator::ArraystdSSvectorLGameObjectPROperators::Set,
            &ArrayReflectionOperator::ArraystdSSvectorLGameObjectPROperators::Get,
            &ArrayReflectionOperator::ArraystdSSvectorLGameObjectPROperators::GetSize,
            &ArrayReflectionOperator::ArraystdSSvectorLGameObjectPROperators::GetArrayTypeName,
            &ArrayReflectionOperator::ArraystdSSvectorLGameObjectPROperators::GetElementTypeName);
        REGISTER_ARRAY_TO_MAP("std::vector<GameObject*>", array_tuple_stdSSvectorLGameObjectPR);
		
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

