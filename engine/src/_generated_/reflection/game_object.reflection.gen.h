#pragma once
#include "runtime/core/reflection/reflection_core.h"
#include "runtime/core/framework/game_object/game_object.h"

namespace peanut{
    class GameObject;
namespace reflection{
namespace TypeReflectionOparator{
    class GameObjectOperators{
    public:
        static const char* GetClassName(){ return "GameObject";}
        static void* ConstructFromJson(const Json& json_context){
            GameObject* ret_instance= new GameObject;
            Serializer::Read(json_context, *ret_instance);
            return ret_instance;
        }
        static Json WriteByName(void* instance){
            return Serializer::Write(*(GameObject*)instance);
        }
        // base class
        static int GetGameObjectBaseClassReflectionInstanceList(ReflectionInstance* &out_list, void* instance){
            int count = 0;
            
            return count;
        }
        // fields
        static const char* GetFieldName_name_(){ return "name_";}
        static const char* GetFieldTypeName_name_(){ return "std::string";}
        static void Set_name_(void* instance, void* field_value){ static_cast<GameObject*>(instance)->name_ = *static_cast<std::string*>(field_value);}
        static void* Get_name_(void* instance){ return static_cast<void*>(&(static_cast<GameObject*>(instance)->name_));}
        static bool IsArray_name_(){ return false; }
        static const char* GetFieldName_guid_(){ return "guid_";}
        static const char* GetFieldTypeName_guid_(){ return "GO_Guid";}
        static void Set_guid_(void* instance, void* field_value){ static_cast<GameObject*>(instance)->guid_ = *static_cast<GO_Guid*>(field_value);}
        static void* Get_guid_(void* instance){ return static_cast<void*>(&(static_cast<GameObject*>(instance)->guid_));}
        static bool IsArray_guid_(){ return false; }
        static const char* GetFieldName_parent_level_(){ return "parent_level_";}
        static const char* GetFieldTypeName_parent_level_(){ return "std::weak_ptr<Level>";}
        static void Set_parent_level_(void* instance, void* field_value){ static_cast<GameObject*>(instance)->parent_level_ = *static_cast<std::weak_ptr<Level>*>(field_value);}
        static void* Get_parent_level_(void* instance){ return static_cast<void*>(&(static_cast<GameObject*>(instance)->parent_level_));}
        static bool IsArray_parent_level_(){ return false; }
        static const char* GetFieldName_components_(){ return "components_";}
        static const char* GetFieldTypeName_components_(){ return "std::vector<Component*>";}
        static void Set_components_(void* instance, void* field_value){ static_cast<GameObject*>(instance)->components_ = *static_cast<std::vector<Component*>*>(field_value);}
        static void* Get_components_(void* instance){ return static_cast<void*>(&(static_cast<GameObject*>(instance)->components_));}
        static bool IsArray_components_(){ return true; }

        // methods
        static const char* GetMethodName_GetName(){ return "GetName";}
        static void Invoke_GetName(void * instance){static_cast<GameObject*>(instance)->GetName();}
        static const char* GetMethodName_GetGuid(){ return "GetGuid";}
        static void Invoke_GetGuid(void * instance){static_cast<GameObject*>(instance)->GetGuid();}
    };
}//namespace TypeReflectionOparator
namespace ArrayReflectionOperator{
#ifndef ArraystdSSvectorLComponentPROperatorMACRO
#define ArraystdSSvectorLComponentPROperatorMACRO
    class ArraystdSSvectorLComponentPROperators{
        public:
            static const char* GetArrayTypeName(){ return "std::vector<Component*>";}
            static const char* GetElementTypeName(){ return "Component*";}
            static int GetSize(void* instance){
                //todo: should check validation
                return static_cast<int>(static_cast<std::vector<Component*>*>(instance)->size());
            }
            static void* Get(int index,void* instance){
                //todo: should check validation
                return static_cast<void*>(&((*static_cast<std::vector<Component*>*>(instance))[index]));
            }
            static void Set(int index, void* instance, void* element_value){
                //todo: should check validation
                (*static_cast<std::vector<Component*>*>(instance))[index] = *static_cast<Component**>(element_value);
            }
    };
#endif //ArraystdSSvectorLComponentPROperator
}//namespace ArrayReflectionOperator

    void TypeWrapperRegister_GameObject(){
		FieldFunctions* field_function_tuple_name_=new FieldFunctions(
            &TypeReflectionOparator::GameObjectOperators::Set_name_,
            &TypeReflectionOparator::GameObjectOperators::Get_name_,
            &TypeReflectionOparator::GameObjectOperators::GetClassName,
            &TypeReflectionOparator::GameObjectOperators::GetFieldName_name_,
            &TypeReflectionOparator::GameObjectOperators::GetFieldTypeName_name_,
            &TypeReflectionOparator::GameObjectOperators::IsArray_name_);
        REGISTER_FIELD_TO_MAP("GameObject", field_function_tuple_name_);
		FieldFunctions* field_function_tuple_guid_=new FieldFunctions(
            &TypeReflectionOparator::GameObjectOperators::Set_guid_,
            &TypeReflectionOparator::GameObjectOperators::Get_guid_,
            &TypeReflectionOparator::GameObjectOperators::GetClassName,
            &TypeReflectionOparator::GameObjectOperators::GetFieldName_guid_,
            &TypeReflectionOparator::GameObjectOperators::GetFieldTypeName_guid_,
            &TypeReflectionOparator::GameObjectOperators::IsArray_guid_);
        REGISTER_FIELD_TO_MAP("GameObject", field_function_tuple_guid_);
		FieldFunctions* field_function_tuple_parent_level_=new FieldFunctions(
            &TypeReflectionOparator::GameObjectOperators::Set_parent_level_,
            &TypeReflectionOparator::GameObjectOperators::Get_parent_level_,
            &TypeReflectionOparator::GameObjectOperators::GetClassName,
            &TypeReflectionOparator::GameObjectOperators::GetFieldName_parent_level_,
            &TypeReflectionOparator::GameObjectOperators::GetFieldTypeName_parent_level_,
            &TypeReflectionOparator::GameObjectOperators::IsArray_parent_level_);
        REGISTER_FIELD_TO_MAP("GameObject", field_function_tuple_parent_level_);
		FieldFunctions* field_function_tuple_components_=new FieldFunctions(
            &TypeReflectionOparator::GameObjectOperators::Set_components_,
            &TypeReflectionOparator::GameObjectOperators::Get_components_,
            &TypeReflectionOparator::GameObjectOperators::GetClassName,
            &TypeReflectionOparator::GameObjectOperators::GetFieldName_components_,
            &TypeReflectionOparator::GameObjectOperators::GetFieldTypeName_components_,
            &TypeReflectionOparator::GameObjectOperators::IsArray_components_);
        REGISTER_FIELD_TO_MAP("GameObject", field_function_tuple_components_);

        MethodFunctions* method_function_tuple_GetName=new MethodFunctions(
            &TypeReflectionOparator::GameObjectOperators::GetMethodName_GetName,
            &TypeReflectionOparator::GameObjectOperators::Invoke_GetName);
        REGISTER_METHOD_TO_MAP("GameObject", method_function_tuple_GetName);
        MethodFunctions* method_function_tuple_GetGuid=new MethodFunctions(
            &TypeReflectionOparator::GameObjectOperators::GetMethodName_GetGuid,
            &TypeReflectionOparator::GameObjectOperators::Invoke_GetGuid);
        REGISTER_METHOD_TO_MAP("GameObject", method_function_tuple_GetGuid);
        
		ArrayFunctions* array_tuple_stdSSvectorLComponentPR = new  ArrayFunctions(
            &ArrayReflectionOperator::ArraystdSSvectorLComponentPROperators::Set,
            &ArrayReflectionOperator::ArraystdSSvectorLComponentPROperators::Get,
            &ArrayReflectionOperator::ArraystdSSvectorLComponentPROperators::GetSize,
            &ArrayReflectionOperator::ArraystdSSvectorLComponentPROperators::GetArrayTypeName,
            &ArrayReflectionOperator::ArraystdSSvectorLComponentPROperators::GetElementTypeName);
        REGISTER_ARRAY_TO_MAP("std::vector<Component*>", array_tuple_stdSSvectorLComponentPR);
		
		ClassFunctions* class_function_tuple_GameObject=new ClassFunctions(
            &TypeReflectionOparator::GameObjectOperators::ConstructFromJson,
            &TypeReflectionOparator::GameObjectOperators::WriteByName,
			&TypeReflectionOparator::GameObjectOperators::GetGameObjectBaseClassReflectionInstanceList);
        REGISTER_BASE_CLASS_TO_MAP("GameObject", class_function_tuple_GameObject);
    }
namespace TypeWrappersRegister{
    void GameObject()
    {
        TypeWrapperRegister_GameObject();
    }
}//namespace TypeWrappersRegister

}//namespace reflection
}//namespace peanut

