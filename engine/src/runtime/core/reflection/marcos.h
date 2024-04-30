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
#if define(__REFLECTION_PARSER__)
#define META_CLASS(...) REFLECTION_CLASS(meta_class, ...)
#define REFLECTION_CLASS(meta_class, ...) __attribute__((annotate(#__VA_ARGS__)))

#define META_STRUCT(...) REFLECTION_CLASS(meta_struct, ...)
#define REFLECTION_STRUCT(meta_struct, ...) __attribute__((annotate(#__VA_ARGS__)))

#define META_PROPERTY(...) REFLECTION_PROPERTY(meta_property, ...)
#define REFLECTION_PROPERTY(meta_property, ...) __attribute__((annotate(#__VA_ARGS__)))

#define META_FUNCTION(...) REFLECTION_FUNCTION(meta_function, ...)
#define REFLECTION_FUNCTION(meta_function, ...) __attribute__((annotate(#__VA_ARGS__)))

#define META_ENUM(...) REFLECTION_FUNCTION(meta_enum, ...)
#define REFLECTION_FUNCTION(meta_enum, ...) __attribute__((annotate(#__VA_ARGS__)))

#else

#define META_CLASS(...) 
#define META_STRUCT(...) 
#define META_PROPERTY(...) 
#define META_FUNCTION(...) 
#define META_ENUM(...)

#endif

#define PEANUT_REFLECTION_NEW(name, ...) reflection::ReflectionPtr(#name, new name(__VA_ARGS__))

#define PEANUT_REFLECTION_DELETE(value) \
	if(value) \
	{			\
		delete value.operator->(); \
		value.GetPtrReference = nullptr; \
	}

#define PEANUT_REFLECTION_DEEP_COPY(type, dst_ptr, src_ptr) \
	*static_cast<type*>(dst_ptr) = *static_cast<type*>(src_ptr.GetPtr);

