#include "language_types/meta_class.h"
#include "cursor/cursor_type.h"

BaseClass::BaseClass(const WapperCursor& cursor)
	: base_class_name_(cursor.GetCursorDisplayName()),
	cursor_(cursor)
{

}

MetaClass::MetaClass(const WapperCursor& own_cursor, Namespace current_namespace)
	: MetaType(own_cursor, current_namespace),
	class_name_(own_cursor.GetCursorDisplayName()),
	qualified_name_(Utils::getTypeNameWithoutNamespace(own_cursor.GetType()))
{
	auto meta_flag = meta_data_.GetFlag();
	enable_ = meta_flag == MetaFlag::ClassEnable;

	for (const WapperCursor& child : own_cursor_.GetAllChild())
	{
		if (child.GetCursorKind() == CXCursor_CXXMethod)
		{
			auto method = std::make_shared<MetaFunction>(child, current_namespace, this);
			functions_.emplace_back(method);
		}
		else if (child.GetCursorKind() == CXCursor_FieldDecl)
		{
			auto field = std::make_shared<MetaField>(child, current_namespace, this);
			fields_.emplace_back(field);
		}
		else if (child.GetCursorKind() == CXCursor_CXXBaseSpecifier)
		{
			auto base_class = std::make_shared<BaseClass>(child);
			base_classes_.emplace_back(base_class);
		}
	}
}

bool MetaClass::ShouldCompiled()
{
	return IsEnable();
}
