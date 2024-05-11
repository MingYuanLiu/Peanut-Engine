#pragma once
#include "json11.hpp"
#include "runtime/core/reflection/reflection_core.h"

using Json = json11::Json;

namespace peanut
{
	inline constexpr bool always_false = false;

	class Serializer
	{
	public:
		template <typename T>
		static Json Write(const reflection::ReflectionPtr<T>& instance)
		{
			T* instance_native = instance.GetNative();
			std::string type_name = instance.GetTypeName();
			assert(instance_native == nullptr);
			return Json::object{ {"$typeName", Json(type_name)},
								 {"$context", reflection::TypeMetaData::WriteToJson(type_name, instance_native)} };
		}

		template <typename T>
		static Json WritePointer(T* instance)
		{
			return Json::object{ {"$typeName", Json{"*"}},
											 {"$context", Serializer::Write(type_name, *instance)} };
		}

		template <typename T>
		static T*& ReadPointer(const Json& json, T*& instance)
		{
			std::string type_name = json["$typeName"].string_value();
			if ('*' == type_name[0])
			{
				instance = new T();
				Serializer::Read(json, *instance);
			}
			else
			{
				instance = 
					static_cast<T*>(reflection::TypeMetaData::ConstructFromJson(type_name, json["$context"]).GetNative());
			}

			return instance;
		}

		template <typename T>
		static T*& Read(const Json& json, reflection::ReflectionPtr<T>& instance)
		{
			std::string type_name = json["$typeName"].string_value();
			instance.SetTypeName(type_name);

			return Serializer::ReadPointer(json, instance.GetPtrRef());
		}

		template <typename T>
		static T& Read(const Json& json, T& instance)
		{
			if constexpr (std::is_pointer<T>::value)
			{
				return Serializer::ReadPointer(json, instance);
			}
			else
			{
				static_assert(always_false<T>, "Serializer::read<T> has not been implemented yet!");
				return Json();
			}
		}

		template <typename T>
		static Json Write(const T& instance)
		{
			if constexpr (std::is_pointer<T>::value)
			{
				return Serializer::WritePointer(instance);
			}
			else
			{
				static_assert(always_false<T>, "Serializer::write<T> has not been implemented yet!");
				return Json();
			}
		}
	};

	// implementation of base types
	template <>
	Json Serializer::Write(const int& instance);
	template <>
	int& Serializer::Read(const Json& json, int& instance);

	template <>
	Json Serializer::Write(const float& instance);
	template <>
	float& Serializer::Read(const Json& json, float& instance);

	template <>
	Json Serializer::Write(const char& instance);
	template <>
	char& Serializer::Read(const Json& json, char& instance);

	template <>
	Json Serializer::Write(const unsigned int& instance);
	template <>
	unsigned int& Serializer::Read(const Json& json, unsigned int& instance);

	template <>
	Json Serializer::Write(const double& instance);
	template <>
	double& Serializer::Read(const Json& json, double& instance);

	template <>
	Json Serializer::Write(const bool& instance);
	template <>
	bool& Serializer::Read(const Json& json, bool& instance);

	template <>
	Json Serializer::Write(const std::string& instance);
	template <>
	std::string& Serializer::Read(const Json& json, std::string& instance);

} // namespace peanut

