#pragma once

#include "language_types/meta_type.h"
#include "language_types/meta_class.h"

class MetaFunction : public MetaType
{
public:
	MetaFunction(const WapperCursor& own_cursor, Namespace current_namespace,
		MetaClass* parent_class = nullptr);

	virtual bool ShouldCompiled() override;

private:
	MetaClass* parent_class_;
};