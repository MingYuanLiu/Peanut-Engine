#include "runtime/core/reflection/reflection_core.h"

namespace peanut {
	namespace reflection
	{
		static std::multimap<std::string, FieldFunctions*> GFieldTypeMap;
		static std::multimap<std::string, MethodFunctions*> GMethodTypeMap;
		static std::map<std::string, ClassFunctions*> GClassTypeMap;
		static std::map<std::string, ArrayFunctions*> GArrayTypeMap;


		static std::string kUnkownTypeName = "UnkownType";

		void TypeMetaDataRegisterInterface::RegisterToFieldMap(const char* name, FieldFunctions* value)
		{
			if (GFieldTypeMap.find(name) == GFieldTypeMap.end())
			{
				GFieldTypeMap.insert(std::make_pair(name, value));
			}
			else
			{
				delete value;
				value = nullptr;
			}

		}

		void TypeMetaDataRegisterInterface::RegisterToMethodMap(const char* name, MethodFunctions* value)
		{
			if (GMethodTypeMap.find(name) == GMethodTypeMap.end())
			{
				GMethodTypeMap.insert(std::make_pair(name, value));
			}
			else
			{
				delete value;
				value = nullptr;
			}
		}

		void TypeMetaDataRegisterInterface::RegisterToClassMap(const char* name, ClassFunctions* value)
		{
			if (GClassTypeMap.find(name) == GClassTypeMap.end())
			{
				GClassTypeMap.insert(std::make_pair(name, value));
			}
			else
			{
				delete value;
				value = nullptr;
			}
		}

		void TypeMetaDataRegisterInterface::RegisterToArrayMap(const char* name, ArrayFunctions* value)
		{
			if (GArrayTypeMap.find(name) == GArrayTypeMap.end())
			{
				GArrayTypeMap.insert(std::make_pair(name, value));
			}
			else
			{
				delete value;
				value = nullptr;
			}
		}

		void TypeMetaDataRegisterInterface::UnregisterAllMap()
		{
			GFieldTypeMap.clear();
			GMethodTypeMap.clear();
			GClassTypeMap.clear();
			GArrayTypeMap.clear();
		}

		TypeMetaData::TypeMetaData() : type_name_(kUnkownTypeName), is_valid_(false)
		{
			fields_.clear();
			methods_.clear();
		}

		TypeMetaData::TypeMetaData(const char* type_name) : type_name_(type_name)
		{
			is_valid_ = false;
			fields_.clear();
			methods_.clear();

			auto field_iter = GFieldTypeMap.equal_range(type_name);
			while (field_iter.first != field_iter.second)
			{
				FieldAccessor field_accessor(field_iter.first->second);
				fields_.emplace_back(field_accessor);

				is_valid_ = true;

				++field_iter.first;
			}

			auto method_iter = GMethodTypeMap.equal_range(type_name);
			while (method_iter.first != method_iter.second)
			{
				MethodAccessor method_accessor(method_iter.first->second);
				methods_.emplace_back(method_accessor);
				is_valid_ = true;

				++method_iter.first;
			}
		}

		TypeMetaData TypeMetaData::NewMetaFromName(std::string type_name)
		{
			return TypeMetaData(type_name.c_str());
		}

		bool TypeMetaData::NewArrayAccessorFromName(std::string type_name, ArrayAccessor& out_array_accessor)
		{
			auto array_iter = GArrayTypeMap.find(type_name);
			if (array_iter != GArrayTypeMap.end())
			{
				out_array_accessor = ArrayAccessor(array_iter->second);
				return true;
			}

			return false;
		}

		json11::Json TypeMetaData::WriteToJson(std::string type_name, void* instance)
		{
			auto iter = GClassTypeMap.find(type_name);
			if (iter != GClassTypeMap.end())
			{
				return std::get<1>(*(iter->second))(instance);
			}

			return json11::Json();
		}

		ReflectionInstance TypeMetaData::ConstructFromJson(std::string type_name, const json11::Json& json_context)
		{
			auto iter = GClassTypeMap.find(type_name);
			if (iter != GClassTypeMap.end())
			{
				TypeMetaData meta(type_name.c_str());
				return ReflectionInstance(meta, std::get<0>(*(iter->second))(json_context));
			}

			return ReflectionInstance();
		}

		// Field Accessor
		FieldAccessor::FieldAccessor() : 
			functions_(nullptr), field_name_(kUnkownTypeName), field_type_name_(kUnkownTypeName)
		{
		}

		FieldAccessor::FieldAccessor(FieldFunctions* functions) : functions_(functions)
		{
			if (functions_ != nullptr)
			{
				field_name_ = std::get<3>(*functions_)();
				field_type_name_ = std::get<4>(*functions_)();
			}
		}

		void FieldAccessor::Set(void* instance, void* value)
		{
			if (functions_ != nullptr)
			{
				std::get<0>(*functions_)(instance, value);
			}
		}

		void* FieldAccessor::Get(void* instance)
		{
			if (functions_ != nullptr)
			{
				return static_cast<void*>(std::get<1>(*functions_)(instance));
			}

			return nullptr;
		}

		TypeMetaData FieldAccessor::GetOwnerTypeMetaData()
		{
			if (functions_ != nullptr)
			{
				TypeMetaData meta(std::get<2>(*functions_)());
				return meta;
			}

			return TypeMetaData(kUnkownTypeName.c_str());
		}

		bool FieldAccessor::GetTypeMetaData(TypeMetaData& out_meta_data)
		{
			if (functions_ != nullptr)
			{
				TypeMetaData meta(field_type_name_.c_str());
				out_meta_data = meta;
				return meta.is_valid_;
			}

			return false;
		}

		bool FieldAccessor::IsArrayType()
		{
			if (functions_ != nullptr)
			{
				return std::get<5>(*functions_)();
			}
		}

		MethodAccessor::MethodAccessor() : functions_(nullptr), method_name_(kUnkownTypeName)
		{
		}

		MethodAccessor::MethodAccessor(MethodFunctions* functions) : 
			functions_(functions),
			method_name_(kUnkownTypeName)
		{
			if (functions_ != nullptr)
			{
				method_name_ = std::get<0>(*functions_)();
			}
		}

		void MethodAccessor::InvokeMethod(void* instance)
		{
			if (functions_ != nullptr)
			{
				std::get<1>(*functions_)(instance);
			}
		}

	} // namespace reflection
} // namespace peanut