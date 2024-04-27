#pragma once

class MetaField
{
public:
	MetaField(const WapperCursor& own_cursor, Namespace current_namespace,
		MetaClass* parent_class = nullptr);

private:
	MetaClass* parent_class_;
};
