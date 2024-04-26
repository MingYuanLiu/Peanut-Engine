#pragma once
#include "common/precompiled.h"

#include <vector>
/**
* The wapper class of CXCursor
*/
class WapperCursor
{
public:
	typedef std::vector<WapperCursor> List;

	WapperCursor(const CXCursor& cursor);
	~WapperCursor() {}

	inline CXCursor GetCursor() const { return cursor_; }

	List GetAllChild();

	std::string GetCursorSpelling() const;

	std::string GetCursorDisplayName() const;

	CXCursorKind GetCursorKind() const { return clang_getCursorKind(cursor_); }

	std::string GetCursorLocateFile();

	void GetCursorLocateLine(uint32_t& start_line, uint32_t& end_line);

	void VisitAllChild(CXCursorVisitor visitor, void* data);

private:
	CXCursor cursor_;
};
