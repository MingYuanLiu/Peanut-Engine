#include "meta_enum.h"

MetaEnum::MetaEnum(const WapperCursor& own_cursor, Namespace current_namespace)
	: MetaType(own_cursor, current_namespace),
	name_(own_cursor.GetCursorDisplayName())
{
	auto flag = meta_data_.GetFlag();
	enable_ = flag == MetaFlag::EnumEnable;

	for (const auto& child : own_cursor.GetAllChild())
	{
		if (child.GetCursorKind() == CXCursor_EnumConstantDecl)
		{
			fields_.emplace_back(std::make_shared<MetaField>(child, current_namespace_));
		}
	}
}


