#pragma once

#define REFLECTION_BODY(class_name) \
	friend class relection::TypeReflectionFieldOperators::Reflection##class_name##Operators;

#define REFLECTION_TYPE(class_name) \
namespace relection { \
	namespace TypeReflectionFieldOperators { \
		class Reflection##class_name##Operators; \
	} \
};

// for reflection parse
#define META_CLASS(class_name, ...) class class_name
#define META_STRUCT(struct_name, ...) struct struct_name
#define PROPERTY(...)

#define PEANUT_REFLECTION_NEW(name, ...) reflection::ReflectionPtr(#name, new name(__VA_ARGS__))

#define PEANUT_REFLECTION_DELETE(value) \
	if(value) \
	{			\
		delete value.operator->(); \
		value.GetPtrReference = nullptr; \
	}

#define PEANUT_REFLECTION_DEEP_COPY(type, dst_ptr, src_ptr) \
	*static_cast<type*>(dst_ptr) = *static_cast<type*>(src_ptr.GetPtr);

