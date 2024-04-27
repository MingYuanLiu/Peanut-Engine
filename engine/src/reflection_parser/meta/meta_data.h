#pragma once
#include "cursor/cursor_wapper.h"

/**
* Meta data of type macro definition
*/
class MetaData
{
public:
	typedef std::vector<std::string> PropertyList;

public:
	MetaData() {}
	explicit MetaData(const WapperCursor& cursor);

	~MetaData() {}

	bool GetPropertyList(PropertyList& out_list) const;

	// flag to indicate the type of macro
	std::string GetFlag() const;

private:
	PropertyList properties_;
};