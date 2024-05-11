#include "runtime/core/serialize/serializer.h"

namespace peanut {
	template <>
	Json Serializer::Write(const int& instance)
	{
		return Json(instance);
	}

	template <>
	Json Serializer::Write(const float& instance)
	{
		return Json(instance);
	}

	template <>
	Json Serializer::Write(const char& instance)
	{
		return Json(instance);
	}

	template <>
	Json Serializer::Write(const unsigned int& instance)
	{
		return Json(static_cast<int>(instance));
	}

	template <>
	Json Serializer::Write(const double& instance)
	{
		return Json(instance);
	}

	template <>
	Json Serializer::Write(const bool& instance)
	{
		return Json(instance);
	}

	template <>
	Json Serializer::Write(const std::string& instance)
	{
		return Json(instance);
	}

	template <>
	int& Serializer::Read(const Json& json, int& instance)
	{
		assert(json.is_number());
		return instance = json.number_value();
	}

	template <>
	float& Serializer::Read(const Json& json, float& instance)
	{
		assert(json.is_number());
		return instance = json.number_value();
	}

	template <>
	char& Serializer::Read(const Json& json, char& instance)
	{
		assert(json.is_number());
		return instance = json.number_value();
	}

	template <>
	unsigned int& Serializer::Read(const Json& json, unsigned int& instance)
	{
		assert(json.is_number());
		return instance = json.number_value();
	}

	template <>
	double& Serializer::Read(const Json& json, double& instance)
	{
		assert(json.is_number());
		return instance = json.number_value();
	}

	template <>
	bool& Serializer::Read(const Json& json, bool& instance)
	{
		assert(json.is_bool());
		return instance = json.bool_value();
	}

	template <>
	std::string& Serializer::Read(const Json& json, std::string& instance)
	{
		assert(json.is_string());
		return instance = json.string_value();
	}
}
