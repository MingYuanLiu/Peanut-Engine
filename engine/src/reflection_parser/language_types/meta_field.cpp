#include "meta_field.h"
#include "language_types/meta_class.h"

#include <string>

MetaField::MetaField(const WapperCursor& own_cursor, Namespace current_namespace, MetaClass* parent_class)
	: MetaType(own_cursor, current_namespace),
	parent_class_(parent_class),
	is_const_(own_cursor.GetType().IsConst()),
	type_(own_cursor_.GetType().GetDisplayName())
{
	auto meta_flag = meta_data_.GetFlag();
	enable_ = meta_flag == MetaFlag::PropertyEnable;

	Utils::ReplaceAll(type_, " ", "");

	std::string namespace_str = CurrentEngineNamespace::engine_namespace;
	namespace_str += "::";
	Utils::ReplaceAll(type_, namespace_str, "");
}

bool MetaField::ShouldCompiled()
{
	return parent_class_ != nullptr ? (parent_class_->ShouldCompiled() && enable_) : false;
}

bool MetaField::IsVector() const
{
	static const std::string vector_prefix = "std::vector<";

	auto index = type_.find(vector_prefix);
	return index != std::string::npos;
}


