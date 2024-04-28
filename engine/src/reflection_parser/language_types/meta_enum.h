#pragma once

#include "meta_type.h"
#include "meta_field.h"

class  MetaEnum : public MetaType
{
public:
	MetaEnum(const WapperCursor& own_cursor, Namespace current_namespace);

	std::string GetEnumName() const { return name_; }

	SharedPtrVector<MetaField> GetFields() {}

private:
	std::string name_;

	SharedPtrVector<MetaField> fields_;
};