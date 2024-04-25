#include "cursor_wapper.h"

WapperCursor::WapperCursor(const CXCursor& cursor) : cursor_(cursor)
{

}

std::string WapperCursor::GetCursorSpelling() const
{
	std::string spelling;
	Utils::toString(clang_getCursorSpelling(cursor_), spelling);

	return spelling;
}

std::string WapperCursor::GetCursorDisplayName() const
{
	std::string displayName;
	Utils::toString(clang_getCursorDisplayName(cursor_), displayName);

	return displayName;
}

std::string WapperCursor::GetCursorLocateFile()
{
	auto cursor_range = clang_Cursor_getSpellingNameRange(cursor_, 0, 0);

	auto start_location = clang_getRangeStart(cursor_range);

	CXFile file;
	uint32_t line, column, offset;

	clang_getFileLocation(start_location, &file, &line, &column, &offset);

	std::string localtion_file;
	Utils::toString(clang_getFileName(file), localtion_file);

	std::cout << "========== GetCursorLocateFile ==============" << std::endl;
	std::cout << "get localtion file: " << localtion_file << std::endl;

	return localtion_file;
}

void WapperCursor::GetCursorLocateLine(uint32_t& start_line, uint32_t& end_line)
{
	auto cursor_range = clang_getCursorExtent(cursor_);

	CXFile file;
	uint32_t start_column, start_offset;
	uint32_t end_column, end_offset;

	clang_getExpansionLocation(clang_getRangeStart(cursor_range), &file, &start_line, &start_column, &start_offset);
	clang_getExpansionLocation(clang_getRangeEnd(cursor_range), &file, &end_line, &end_column, &end_offset);

	std::string localtion_file;
	Utils::toString(clang_getFileName(file), localtion_file);

	std::cout << "========== GetCursorLocateLine ==============" << std::endl;
	std::cout << "get localtion file: " << localtion_file << std::endl;
	std::cout << "start_line: " << start_line << " end_line: " << end_line << std::endl;
	std::cout << "start_column: " << start_column << " end_column: " << end_column << std::endl;
	std::cout << "=============================================" << std::endl;
}
