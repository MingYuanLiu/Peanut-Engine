#pragma once

namespace peanut
{
// for reflection parse
#if defined(__REFLECTION_PARSER__)

#define REFLECTION_CLASS(...) __attribute__((annotate(#__VA_ARGS__)))
#define META_CLASS(...) REFLECTION_CLASS(meta_class, __VA_ARGS__)

#define REFLECTION_STRUCT(...) __attribute__((annotate(#__VA_ARGS__)))
#define META_STRUCT(...) REFLECTION_STRUCT(meta_struct, __VA_ARGS__)

#define REFLECTION_PROPERTY(...) __attribute__((annotate(#__VA_ARGS__)))
#define META_PROPERTY(...) REFLECTION_PROPERTY(meta_property, __VA_ARGS__)

#define REFLECTION_FUNCTION(...) __attribute__((annotate(#__VA_ARGS__)))
#define META_FUNCTION(...) REFLECTION_FUNCTION(meta_function, __VA_ARGS__)

#define REFLECTION_ENUM(...) __attribute__((annotate(#__VA_ARGS__)))
#define META_ENUM(...) REFLECTION_ENUM(meta_enum, __VA_ARGS__)

#else

#define META_CLASS(...)
#define META_STRUCT(...)
#define META_PROPERTY(...)
#define META_FUNCTION(...)
#define META_ENUM(...)

#endif

//#define REFLECTION_BODY(class_name) \
//	friend class relection::TypeReflectionFieldOperators::Reflection##class_name##Operators;
//
//#define REFLECTION_TYPE(class_name) \
//namespace relection { \
//	namespace TypeReflectionFieldOperators { \
//		class Reflection##class_name##Operators; \
//	} \
//};
//
//#define PEANUT_REFLECTION_NEW(name, ...) reflection::ReflectionPtr(#name, new name(__VA_ARGS__))
//
//#define PEANUT_REFLECTION_DELETE(value) \
//	if(value) \
//	{			\
//		delete value.operator->(); \
//		value.GetPtrReference = nullptr; \
//	}
//
//#define PEANUT_REFLECTION_DEEP_COPY(type, dst_ptr, src_ptr) \
//	*static_cast<type*>(dst_ptr) = *static_cast<type*>(src_ptr.GetPtr);

} // namespace peanut
