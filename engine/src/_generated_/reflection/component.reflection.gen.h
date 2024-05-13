#pragma once
#include "runtime/core/reflection/reflection_core.h"
#include "runtime/core/framework/component/component.h"

namespace peanut{
    class Component;
namespace reflection{
namespace TypeReflectionOparator{
    class ComponentOperators{
    public:
        static const char* GetClassName(){ return "Component";}
        static void* ConstructFromJson(const Json& json_context){
            Component* ret_instance= new Component;
            Serializer::Read(json_context, *ret_instance);
            return ret_instance;
        }
        static Json WriteByName(void* instance){
            return Serializer::Write(*(Component*)instance);
        }
        // base class
        static int GetComponentBaseClassReflectionInstanceList(ReflectionInstance* &out_list, void* instance){
            int count = 0;
            
            return count;
        }
        // fields
        static const char* GetFieldName_parent_(){ return "parent_";}
        static const char* GetFieldTypeName_parent_(){ return "int";}
        static void Set_parent_(void* instance, void* field_value){ static_cast<Component*>(instance)->parent_ = *static_cast<int*>(field_value);}
        static void* Get_parent_(void* instance){ return static_cast<void*>(&(static_cast<Component*>(instance)->parent_));}
        static bool IsArray_parent_(){ return false; }
        static const char* GetFieldName_is_dirty_(){ return "is_dirty_";}
        static const char* GetFieldTypeName_is_dirty_(){ return "bool";}
        static void Set_is_dirty_(void* instance, void* field_value){ static_cast<Component*>(instance)->is_dirty_ = *static_cast<bool*>(field_value);}
        static void* Get_is_dirty_(void* instance){ return static_cast<void*>(&(static_cast<Component*>(instance)->is_dirty_));}
        static bool IsArray_is_dirty_(){ return false; }

        // methods
        static const char* GetMethodName_Attach(){ return "Attach";}
        static void Invoke_Attach(void * instance){static_cast<Component*>(instance)->Attach();}
        static const char* GetMethodName_Detach(){ return "Detach";}
        static void Invoke_Detach(void * instance){static_cast<Component*>(instance)->Detach();}
        static const char* GetMethodName_BeginPlay(){ return "BeginPlay";}
        static void Invoke_BeginPlay(void * instance){static_cast<Component*>(instance)->BeginPlay();}
        static const char* GetMethodName_EndPlay(){ return "EndPlay";}
        static void Invoke_EndPlay(void * instance){static_cast<Component*>(instance)->EndPlay();}
        static const char* GetMethodName_Tick(){ return "Tick";}
        static void Invoke_Tick(void * instance){static_cast<Component*>(instance)->Tick();}
    };
}//namespace TypeReflectionOparator


    void TypeWrapperRegister_Component(){
		FieldFunctions* field_function_tuple_parent_=new FieldFunctions(
            &TypeReflectionOparator::ComponentOperators::Set_parent_,
            &TypeReflectionOparator::ComponentOperators::Get_parent_,
            &TypeReflectionOparator::ComponentOperators::GetClassName,
            &TypeReflectionOparator::ComponentOperators::GetFieldName_parent_,
            &TypeReflectionOparator::ComponentOperators::GetFieldTypeName_parent_,
            &TypeReflectionOparator::ComponentOperators::IsArray_parent_);
        REGISTER_FIELD_TO_MAP("Component", field_function_tuple_parent_);
		FieldFunctions* field_function_tuple_is_dirty_=new FieldFunctions(
            &TypeReflectionOparator::ComponentOperators::Set_is_dirty_,
            &TypeReflectionOparator::ComponentOperators::Get_is_dirty_,
            &TypeReflectionOparator::ComponentOperators::GetClassName,
            &TypeReflectionOparator::ComponentOperators::GetFieldName_is_dirty_,
            &TypeReflectionOparator::ComponentOperators::GetFieldTypeName_is_dirty_,
            &TypeReflectionOparator::ComponentOperators::IsArray_is_dirty_);
        REGISTER_FIELD_TO_MAP("Component", field_function_tuple_is_dirty_);

        MethodFunctions* method_function_tuple_Attach=new MethodFunctions(
            &TypeReflectionOparator::ComponentOperators::GetMethodName_Attach,
            &TypeReflectionOparator::ComponentOperators::Invoke_Attach);
        REGISTER_METHOD_TO_MAP("Component", method_function_tuple_Attach);
        MethodFunctions* method_function_tuple_Detach=new MethodFunctions(
            &TypeReflectionOparator::ComponentOperators::GetMethodName_Detach,
            &TypeReflectionOparator::ComponentOperators::Invoke_Detach);
        REGISTER_METHOD_TO_MAP("Component", method_function_tuple_Detach);
        MethodFunctions* method_function_tuple_BeginPlay=new MethodFunctions(
            &TypeReflectionOparator::ComponentOperators::GetMethodName_BeginPlay,
            &TypeReflectionOparator::ComponentOperators::Invoke_BeginPlay);
        REGISTER_METHOD_TO_MAP("Component", method_function_tuple_BeginPlay);
        MethodFunctions* method_function_tuple_EndPlay=new MethodFunctions(
            &TypeReflectionOparator::ComponentOperators::GetMethodName_EndPlay,
            &TypeReflectionOparator::ComponentOperators::Invoke_EndPlay);
        REGISTER_METHOD_TO_MAP("Component", method_function_tuple_EndPlay);
        MethodFunctions* method_function_tuple_Tick=new MethodFunctions(
            &TypeReflectionOparator::ComponentOperators::GetMethodName_Tick,
            &TypeReflectionOparator::ComponentOperators::Invoke_Tick);
        REGISTER_METHOD_TO_MAP("Component", method_function_tuple_Tick);
        
        
		
		ClassFunctions* class_function_tuple_Component=new ClassFunctions(
            &TypeReflectionOparator::ComponentOperators::ConstructFromJson,
            &TypeReflectionOparator::ComponentOperators::WriteByName,
			&TypeReflectionOparator::ComponentOperators::GetComponentBaseClassReflectionInstanceList);
        REGISTER_BASE_CLASS_TO_MAP("Component", class_function_tuple_Component);
    }
namespace TypeWrappersRegister{
    void Component()
    {
        TypeWrapperRegister_Component();
    }
}//namespace TypeWrappersRegister

}//namespace reflection
}//namespace peanut

