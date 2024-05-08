#pragma once

#include <json11.hpp>

#include <functional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <tuple>

namespace peanut {


	// declare here used by functions' defination
	namespace reflection
	{
		class TypeDataMeta;
		class FieldAccessor;
		class MethodAccessor;
		class ArrayAccessor;
		class ReflectionInstance;
	} // namespace Reflection

	#define REGISTER_FILED_MAP(name, value) peanut::TypeMetaDataRegisterInterface::RegisterToFieldMap(name, value)
	#define REGISTER_ARRAY_MAP(name, value) peanut::TypeMetaDataRegisterInterface::RegisterToArrayMap(name, value)
	#define REGISTER_BASE_CLASS_MAP(name, value) peanut::TypeMetaDataRegisterInterface::RegisterToClassMap(name, value)
	#define REGISTER_METHOD_MAP(name, value) peanut::TypeMetaDataRegisterInterface::RegisterToMethodMap(name, value)
	#define UNREGISTER_ALL_MAP() peanut::TypeMetaDataRegisterInterface::UnRegisterAllMap()

	#define TypeMetaDefine(class_name, object_ptr_of_class) \
		peanut::reflection::ReflectionInstance(peanut::reflection::TypeDataMeta::NewMetaFromName(#class_name), object_ptr_of_class)

	#define TypeMetaDefinePtr(class_name, object_ptr_of_class) \
		new peanut::reflection::ReflectionInstance(peanut::reflection::TypeDataMeta::NewMetaFromName(#class_name), object_ptr_of_class)

	// define methods
	typedef std::function<void(void*/*instance*/, void*/*filed value*/)> SetFunction;
	typedef std::function<void*/*filed value*/ (void* /*instance*/)>	 GetFunction;
	typedef std::function<const char*/*filed & class name*/()>			 GetNameFunction;
	typedef std::function<bool()>										 GetBoolFunction;

	// feild methods
	typedef std::tuple<SetFunction, GetFunction, GetNameFunction /*class name*/, GetNameFunction /*filed name*/,
		GetNameFunction /*filed type name*/, GetBoolFunction/*is arrary type*/> FieldFunctions;

	// define class functions
	typedef std::function<void*/*instance*/(const json11::Json&)>		  ConstructWithJson;
	typedef std::function<json11::Json(void*)>							  WriteJson;
	typedef std::function<int(reflection::ReflectionInstance*&, void*)>	  GetBaseClassReflectionInstanceListFunction;

	typedef std::tuple<ConstructWithJson, WriteJson, GetBaseClassReflectionInstanceListFunction> ClassFunctions;

	//define method functions
	typedef std::function<void(void*/*method address*/)>				 InvokeFunction;
	typedef std::tuple<GetNameFunction/*method name*/, InvokeFunction> MethodFunctions;

	// define array functions
	typedef std::function<int(void*/*instance*/)>						 GetSizeFuncion;
	typedef std::function<void(void*/*instance*/, void*/*array value*/, int/*index*/)> SetArrayFunction;
	typedef std::function<void* (void*/*instance*/, int/*index*/)>		 GetArrayFuntion;

	typedef std::tuple<SetArrayFunction, GetArrayFuntion, GetSizeFuncion,
		GetNameFunction /*array name*/, GetNameFunction /*value name*/> ArrayFunctions;


	namespace reflection
	{
		// 反射核心类：会将所有类型元数据保存到该类中，
		// 外部用户可通过下面定义的接口可以获取反射对应的变量、方法、类型信息
		// 
		class TypeMetaData
		{
			friend class FieldAccessor;
			friend class MethodAccessor;
			friend class ArrayAccessor;

		public:
			TypeMetaData();

			static TypeMetaData NewMetaFromName(std::string type_name);
			static ReflectionInstance ConstructFromJson(std::string type_name, const json11::Json& json_context);
			static bool NewArrayAccessorFromName(std::string type_name, ArrayAccessor& out_array_accessor);
			static json11::Json WriteToJson(std::string type_name, void* instance);

			std::string GetTypeName() { return type_name_; };

			FieldAccessor GetFieldByName(const std::string& name);
			MethodAccessor GetMethodByName(const std::string& name);

			int GetFieldList(std::vector<FieldAccessor>& out_field_list);
			int GetMethodList(std::vector<MethodAccessor>& out_method_list);

			bool IsValid() { return is_valid_; }

			int GetBaseClassRefectionInstanceList(void* instance, ReflectionInstance*& out_instance_list);

		private:
			explicit TypeMetaData(const char* type_name);

		private:
			std::vector<FieldAccessor, std::allocator<FieldAccessor> > fields_;
			std::vector<MethodAccessor, std::allocator<MethodAccessor> > methods_;
			std::string type_name_;

			// whether current type is already registered
			bool is_valid_;
		};

		class ReflectionInstance
		{
		public:
			ReflectionInstance() {}
			ReflectionInstance(const TypeMetaData& meta_data, void* instance)
				: meta_data_(meta_data), instance_(instance) {}

		private:
			TypeMetaData meta_data_;
			void* instance_;
		};

		class FieldAccessor
		{
		public:
			FieldAccessor();
			explicit FieldAccessor(FieldFunctions* functions);

			void Set(void* instance, void* value);
			void* Get(void* instance);

			TypeMetaData GetOwnerTypeMetaData();
			bool GetTypeMetaData(TypeMetaData& out_meta_data);

			inline const char* GetFieldName() const { return field_name_; }
			inline const char* GetFieldTypeName() const { return field_type_name_; }

			bool IsArrayType();

		private:
			FieldFunctions* functions_;
			const char* field_name_;
			const char* field_type_name_;
		};

		class MethodAccessor
		{
		public:
			MethodAccessor() {}
			explicit MethodAccessor(MethodFunctions* functions) : functions_(functions) {}

		private:
			MethodFunctions* functions_;
		};

		class ArrayAccessor
		{
		public:
			ArrayAccessor() {}
			explicit ArrayAccessor(ArrayFunctions* functions) : functions_(functions) {}

		private:
			ArrayFunctions* functions_;
		};

		template<typename T>
		class ReflectionPtr
		{
		public:
			// constructor
			ReflectionPtr() : instance_(nullptr) {}
		};

		// 反射实现的基本逻辑：
		// 1. 反射解析程序提取出带有反射标记的类型信息；
		// 2. 将提出到的类型信息，注册到记录表中；
		// 3. 使用时，根据类型名去记录表中查找对应的反射信息；
		class TypeMetaDataRegisterInterface
		{
			void RegisterToFieldMap(const char* name, FieldFunctions* value);

			void RegisterToClassMap(const char* name, ClassFunctions* value);

			void RegisterToMethodMap(const char* name, MethodFunctions* value);

			void RegisterToArrayMap(const char* name, ArrayFunctions* value);

			void UnregisterAllMap();
		};
	}

} // namespace peanut