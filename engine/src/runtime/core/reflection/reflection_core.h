#pragma once

#include <json11.hpp>

#include <functional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <tuple>
#include <assert.h>

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

			FieldAccessor GetFieldByName(const std::string& name); // todo: impl
			MethodAccessor GetMethodByName(const std::string& name); // todo: impl

			inline int GetFieldList(std::vector<FieldAccessor>& out_field_list); // todo: impl
			inline int GetMethodList(std::vector<MethodAccessor>& out_method_list); // todo: impl

			bool IsValid() { return is_valid_; }

			int GetBaseClassRefectionInstanceList(void* instance, ReflectionInstance*& out_instance_list); // todo: impl

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
			ReflectionInstance() : meta_data_(TypeMetaData()), instance_(nullptr) {}
			ReflectionInstance(const TypeMetaData& meta_data, void* instance)
				: meta_data_(meta_data), instance_(instance) {}

			ReflectionInstance& operator=(const ReflectionInstance& other);
			ReflectionInstance& operator=(const ReflectionInstance&& other);

		private:
			TypeMetaData meta_data_;
			void* instance_;
		};

		class FieldAccessor
		{
			friend class TypeMetaData;
		public:
			FieldAccessor();

			void Set(void* instance, void* value);
			void* Get(void* instance);

			TypeMetaData GetOwnerTypeMetaData();
			bool GetTypeMetaData(TypeMetaData& out_meta_data);

			inline const char* GetFieldName() const { return field_name_.c_str(); }
			inline const char* GetFieldTypeName() const { return field_type_name_.c_str(); }

			bool IsArrayType();

			FieldAccessor& operator=(const FieldAccessor& other);

		private:
			explicit FieldAccessor(FieldFunctions* functions);

		private:
			FieldFunctions* functions_;
			std::string field_name_;
			std::string field_type_name_;
		};

		class MethodAccessor
		{
			friend class TypeMetaData;
		public:
			MethodAccessor();
			

			inline const char* GetMethodName() const { return method_name_.c_str(); }

			void InvokeMethod(void* instance);

			MethodAccessor& operator=(const MethodAccessor& other);

		private:
			explicit MethodAccessor(MethodFunctions* functions);

		private:
			MethodFunctions* method_functions_; 
			std::string method_name_;
		};

		class ArrayAccessor
		{
			friend class TypeMetaData;
		public:
			ArrayAccessor();
			inline const char* GetArrayName() const { return arrary_name_.c_str(); }
			inline const char* GetElementTypeName() const { return element_type_name_.c_str(); }

			void Set(void* instance, void* value, int index);
			void* Get(void* instance, int index);

			int GetSize(void* instance);

			ArrayAccessor& operator=(const ArrayAccessor& other);

		private:
			explicit ArrayAccessor(ArrayFunctions* functions);

		private:
			ArrayFunctions* array_functions_;
			std::string arrary_name_;
			std::string element_type_name_;
		};

		template<typename T>
		class ReflectionPtr
		{
		public:
			// constructor
			ReflectionPtr(const std::string& type_name, T* instance)
				: type_name_(type_name), instance_(instance) {}

			ReflectionPtr() : type_name_(), instance_(nullptr) {}

			ReflectionPtr(const ReflectionPtr& other)
				: type_name_(other.type_name_), instance_(other.instance_) {}

			std::string GetTypeName() { return type_name_; }
			void SetTypeName(const std::string& type_name) { type_name_ = type_name; }

			T* GetNative() { return instance_; }
			T* GetNative() const { return instance_; }

			T* operator->() { 
				assert(instance_ != nullptr);
				return instance_; 
			}

			T* operator->() const { 
				assert(instance_ != nullptr);
				return instance_; 
			}

			T& operator*() { 
				assert(instance_ != nullptr);
				return *instance_; 
			}

			T& operator*() const { 
				assert(instance_ != nullptr);
				return *instance_; 
			}

			const T& operator*() const { 
				assert(instance_ != nullptr);
				return *(static_cast<const T*>(instance_));
			}

			operator bool() const { return (instance_ != nullptr); }

			ReflectionPtr<T>& operator=(const ReflectionPtr<T>& other)
			{
				if (this == &other)
				{
					return *this;
				}

				type_name_ = other.type_name_;
				instance_ = other.instance_;
				return *this;
			}

			ReflectionPtr<T>& operator=(ReflectionPtr<T>&& other)
			{
				if (this == &other)
				{
					return *this;
				}

				type_name_ = other.type_name_;
				instance_ = other.instance_;
				return *this;
			}

			template<typename U>
			ReflectionPtr<T>& operator=(const ReflectionPtr<U>& other)
			{
				if (this == &other)
				{
					return *this;
				}

				type_name_ = other.GetTypeName();
				instance_ = static_cast<T*>(other.GetNative());
				return *this;
			}

			template<typename U>
			ReflectionPtr<T>& operator=(ReflectionPtr<U>&& other)
			{
				if (this == &other)
				{
					return *this;
				}

				type_name_ = other.GetTypeName();
				instance_ = static_cast<T*>(other.GetNative());
				return *this;
			}

			bool operator==(const ReflectionPtr<T>& rhs_other) const
			{
				return (instance_ == rhs_other.instance_);
			}

			bool operator!=(ReflectionPtr<T>&& rhs_other) const
			{
				return (instance_ != rhs_other.instance_);
			}

			bool operator==(const T* other) const 
			{
				return (instance_ == other);
			}

			bool operator!=(const T* other) const
			{
				return (instance_ != other);
			}

			template<typename T1>
			explicit operator T1*() 
			{
				return static_cast<T1*>(instance_);
			}

			template<typename T1>
			explicit operator const T1*() const
			{
				return static_cast<const T1*>(instance_);
			}

			template<typename T1>
			explicit operator ReflectionPtr<T1>()
			{
				return ReflectionPtr<T1>(type_name_, static_cast<T1*>(instance_));
			}

			template<typename T1>
			explicit operator ReflectionPtr<T1>() const
			{
				return ReflectionPtr<T1>(type_name_, static_cast<T1*>(instance_));
			}

		private:
			std::string type_name_;
			typedef T Type;
			T* instance_;
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