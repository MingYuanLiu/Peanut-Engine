#pragma once

#include "language_types/meta_type.h"
#include "language_types/meta_class.h"

class MetaField : public MetaType
{
public:
	MetaField(const WapperCursor& own_cursor, Namespace current_namespace,
		MetaClass* parent_class = nullptr);

	inline std::string GetDisplayName() const { return own_cursor_.GetCursorDisplayName(); }

	inline std::string GetTypeName() const { return type_; }

	inline bool IsConst() const { return is_const_; }

	bool IsVector() const;

	virtual bool ShouldCompiled() override;

private:
	MetaClass* parent_class_;
	bool is_const_;
	std::string type_;
};
