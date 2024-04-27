#include "meta_type.h"

MetaType::MetaType(const WapperCursor& own_cursor, Namespace current_namespace)
	: meta_data_(MetaData(own_cursor)), 
	current_namespace_(current_namespace),
	enable_(false),
	own_cursor_(own_cursor),
	name_(own_cursor.GetCursorSpelling())
{

}

std::string MetaType::GetSourceFile() const
{
	return own_cursor_.GetCursorLocateFile();
}