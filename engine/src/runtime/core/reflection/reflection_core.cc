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

		FieldAccessor TypeMetaData::GetFieldByName(const std::string& name)
		{
			for (const auto& accessor : fields_)
			{
				if (accessor.GetFieldName() == name)
				{
					return accessor;
				}
			}
		}

		MethodAccessor TypeMetaData::GetMethodByName(const std::string& name)
		{

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

		FieldAccessor& FieldAccessor::operator=(const FieldAccessor& other)
		{
			if (this == &other)
			{
				return *this;
			}

			functions_ = other.functions_;
			field_name_ = other.field_name_;
			field_type_name_ = other.field_type_name_;
			return *this;
		}

		// method
		MethodAccessor::MethodAccessor() : method_functions_(nullptr), method_name_(kUnkownTypeName)
		{
		}

		MethodAccessor::MethodAccessor(MethodFunctions* functions) : 
			method_functions_(functions),
			method_name_(kUnkownTypeName)
		{
			if (method_functions_ != nullptr)
			{
				method_name_ = std::get<0>(*method_functions_)();
			}
		}

		void MethodAccessor::InvokeMethod(void* instance)
		{
			if (method_functions_ != nullptr)
			{
				std::get<1>(*method_functions_)(instance);
			}
		}

		MethodAccessor& MethodAccessor::operator=(const MethodAccessor& other)
		{
			if (this == &other)
			{
				return *this;
			}

			method_functions_ = other.method_functions_;
			method_name_ = other.method_name_;
			return *this;
		}

		// arrary
		ArrayAccessor::ArrayAccessor() : 
			array_functions_(nullptr),
			arrary_name_(kUnkownTypeName),
			element_type_name_(kUnkownTypeName)
		{
		}

		ArrayAccessor::ArrayAccessor(ArrayFunctions* functions) : array_functions_(functions)
		{
			if (array_functions_ != nullptr)
			{
				arrary_name_ = std::get<3>(*array_functions_)();
				element_type_name_ = std::get<4>(*array_functions_)();
			}
		}

		void ArrayAccessor::Set(void* instance, void* value, int index)
		{
			if (array_functions_ != nullptr)
			{
				std::get<0>(*array_functions_)(instance, value, index);
			}
		}

		void* ArrayAccessor::Get(void* instance, int index)
		{
			if (array_functions_ != nullptr)
			{
				return static_cast<void*>(std::get<1>(*array_functions_)(instance, index));
			}

			return nullptr;
		}

		int ArrayAccessor::GetSize(void* instance)
		{
			if (array_functions_ != nullptr)
			{
				return static_cast<int>(std::get<2>(*array_functions_)(instance));
			}

			return 0;
		}

		ArrayAccessor& ArrayAccessor::operator=(const ArrayAccessor& other)
		{
			if (this == &other)
			{
				return *this;
			}

			array_functions_ = other.array_functions_;
			arrary_name_ = other.arrary_name_;
			element_type_name_ = other.element_type_name_;
			return *this;
		}

		ReflectionInstance& ReflectionInstance::operator=(const ReflectionInstance& other)
		{
			if (this == &other)
			{
				return *this;
			}

			meta_data_ = other.meta_data_;
			instance_ = other.instance_;
			return *this;
		}

		ReflectionInstance& ReflectionInstance::operator=(const ReflectionInstance&& other)
		{
			if (this == &other)
			{
				return *this;
			}

			meta_data_ = other.meta_data_;
			instance_ = other.instance_;
			return *this;
		}

	} // namespace reflection
} // namespace peanut