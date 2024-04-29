#include "language_types/meta_function.h"

MetaFunction::MetaFunction(const WapperCursor& own_cursor, Namespace current_namespace, MetaClass* parent_class = nullptr)
	: MetaType(own_cursor, current_namespace),
	parent_class_(parent_class)
{
	auto meta_flag = meta_data_.GetFlag();
	enable_ = meta_flag == MetaFlag::FunctionEnable;
}

bool MetaFunction::ShouldCompiled()
{
	return parent_class_ != nullptr ? (parent_class_->ShouldCompiled() && enable_) : false;
}
